[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class WDS_DurabilityComponentClass : ScriptComponentClass
{
}

class WDS_DurabilityComponent : ScriptComponent
{	
	[Attribute(category: "Destruction", desc: "The model it will swap to when the gun explodes")]
	protected ResourceName m_sDestroyedEntity;
	
	[Attribute(defvalue: "{5592BC9B67C60D16}Particles/Weapon/Explosion_RGD5.ptc", category: "Destruction", desc: "Particle effect used for when gun explodes")]
	protected ResourceName m_sExplosionEffect;
	
	[Attribute(defvalue: "100", category: "Durability Settings")]
	protected float m_fMaximumDurability;
	
	[Attribute(defvalue: "100", category: "Durability Settings")]
	protected float m_fDefaultDurability;
	
	[Attribute(defvalue: "0.10", category: "Durability Settings")]
	protected float m_fDegradePerShot;
	
	[Attribute(defvalue: "30.0", category: "Durability Settings")]
	protected float m_fJamAtDurability;
	
	[Attribute(defvalue: "20.0", category: "Durability Settings")]
	protected float m_fJammingChance;
	
	[RplProp()]
	protected float m_fCurrentDurability = m_fDefaultDurability;
	
	float GetCurrentDurability()
	{
		return m_fCurrentDurability;
	}
	
	float GetMaxDurability()
	{
		return m_fMaximumDurability;
	}
	
	void SetDurability(float value)
	{
		if (value > m_fMaximumDurability)
			value = m_fMaximumDurability;
		
		if (m_fCurrentDurability == value)
			return;
		
		m_fCurrentDurability = value;
		Replication.BumpMe();
	}

	// This is called when the player shoots from player controller
	// Degrades per shot
	void Degrade(int playerId, notnull BaseWeaponComponent weapon)
	{
		m_fCurrentDurability -= m_fDegradePerShot;
		if (m_fCurrentDurability < 0)
			m_fCurrentDurability = 0;
		
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComp && rplComp.IsOwner())
			Replication.BumpMe();
		
		PrintFormat("Durability: %1", m_fCurrentDurability);
		if (m_fCurrentDurability <= 0) {
			PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (playerController) {
				IEntity ent = playerController.GetControlledEntity();
				if (!ent)
					return;
				
				ChimeraCharacter character = ChimeraCharacter.Cast(ent);
				if (!character)
					return;
				
				SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (!characterController)
					return;
				
				// Particle effect?
				if (m_sExplosionEffect) {
					ParticleEffectEntitySpawnParams spawnParams();
					GetOwner().GetTransform(spawnParams.Transform);
					spawnParams.FollowParent = GetOwner();
					spawnParams.PlayOnSpawn = true;
					spawnParams.UseFrameEvent = true;
					spawnParams.DeleteWhenStopped = true;
					ParticleEffectEntity.SpawnParticleEffect(m_sExplosionEffect, spawnParams);
				}
				
				characterController.SetUnconscious(true);
				GetGame().GetCallqueue().CallLater(characterController.SetUnconscious, 10 * 1000, false, false);
				
				// Swap weapon model to something destroyed here?
				if (m_sDestroyedEntity) {
					EntitySpawnParams params();
					params.TransformMode = ETransformMode.LOCAL;
					GetOwner().GetTransform(params.Transform);

					IEntity destroyedGun = GetGame().SpawnEntityPrefab(m_sDestroyedEntity, false, GetOwner().GetWorld(), params);
					if (destroyedGun) {
						SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
						Physics destroyedPhys = destroyedGun.GetPhysics();
						if (destroyedPhys) {
							destroyedPhys.ChangeSimulationState(SimulationState.SIMULATION);
							destroyedPhys.ApplyImpulse(vector.Up * destroyedPhys.GetMass() * 0.001);
						}
					}
				}
			}
		} else {
			if (m_fCurrentDurability < m_fJamAtDurability) {
				float dice = Math.RandomFloat01();
				if (dice <= (m_fJammingChance / 100)) {
					WeaponComponent wepComponent = WeaponComponent.Cast(GetOwner().FindComponent(WeaponComponent));
					if (wepComponent) {
						MuzzleComponent currentMuzzle = MuzzleComponent.Cast(wepComponent.GetCurrentMuzzle());
						if (!currentMuzzle)
							return;
						
						int currentBarrelIndex = currentMuzzle.GetCurrentBarrelIndex();
						
						BaseMagazineComponent currentMagazine = wepComponent.GetCurrentMagazine();
						
						// Clear the chamber to give the illusion the gun "jammed"
						int count = currentMagazine.GetAmmoCount();
						currentMuzzle.ClearChamber(currentBarrelIndex);
						
						// Insert the bullet from chamber into the magazine
						if (currentMagazine)
							currentMagazine.SetAmmoCount(++count);
						
						WeaponSoundComponent soundComp = WeaponSoundComponent.Cast(GetOwner().FindComponent(WeaponSoundComponent));
						if (soundComp) {
							soundComp.SoundEvent("SOUND_RELOAD_BOLT_RELEASE");
						}
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// remove if unused
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// remove if unused
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// remove if unused
		// SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		// remove if unused
	}
}
