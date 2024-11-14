[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class WDS_DurabilityComponentClass : ScriptComponentClass
{
}

class WDS_DurabilityComponent : ScriptComponent
{	
	[Attribute(category: "Destruction", UIWidgets.ResourcePickerThumbnail, desc: "If filled it will swap the gun to the defined prefab when durability reaches 0, you can use it to create destroyed gun effects etc")]
	protected ResourceName m_sDestroyedEntity;
	
	[Attribute(defvalue: "{5592BC9B67C60D16}Particles/Weapon/Explosion_RGD5.ptc", UIWidgets.ResourcePickerThumbnail, params: "ptc", category: "Destruction", desc: "Particle effect used for when gun explodes")]
	protected ResourceName m_sExplosionEffect;
	
	[Attribute(defvalue: "barrel_muzzle", category: "Destruction", desc: "Bone name for attaching the particle too, leave it as default value if you don't know what you are doing.")]
	protected string m_sParticleBoneName;
	
	[Attribute(defvalue: "1", category: "Destruction", desc: "Should the player take damage when weapon 'Explodes'")]
	protected bool m_bDamageOnExplode;
	
	[Attribute(defvalue: "100", category: "Durability Settings")]
	protected float m_fMaximumDurability;
	
	[Attribute(defvalue: "100", category: "Durability Settings")]
	protected float m_fDefaultDurability;
	
	[Attribute(defvalue: "0.10", category: "Durability Settings")]
	protected float m_fDegradePerShot;
	
	[Attribute(defvalue: "25.0", category: "Durability Settings")]
	protected float m_fJamAtDurability;
	
	[Attribute(defvalue: "4.5", category: "Durability Settings")]
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
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void Rpc_Broadcast_CreateParticleEffect(RplId entityId)
	{
		if (!entityId.IsValid())
			return;
		
		WDS_DurabilityComponent durabilityComponent = WDS_DurabilityComponent.Cast(Replication.FindItem(entityId));
		if (!durabilityComponent)
			return;
		
		Animation anm = durabilityComponent.GetOwner().GetAnimation();
		if (!anm)
			return;
		
		ParticleEffectEntitySpawnParams spawnParams();
		spawnParams.Parent = durabilityComponent.GetOwner();
		spawnParams.PlayOnSpawn = true;
		spawnParams.DeleteWhenStopped = true;
		spawnParams.PivotID = anm.GetBoneIndex(m_sParticleBoneName);
		ParticleEffectEntity.SpawnParticleEffect(m_sExplosionEffect, spawnParams);
	}
	
	protected float JamChance()
	{
		#ifdef WORKBENCH
		PrintFormat("Jam Chance: %1", m_fJammingChance);
		#endif
		float thresholdPercentage = m_fJammingChance / 100;
		float thresholdDurability = m_fMaximumDurability * (thresholdPercentage);
		if (m_fCurrentDurability > thresholdDurability)
			return thresholdPercentage;
		else {
			float scalingChance = (thresholdDurability - m_fCurrentDurability) / thresholdDurability;
			float chance = thresholdPercentage + scalingChance * ((m_fJamAtDurability) - m_fJammingChance);
			#ifdef WORKBENCH
			PrintFormat("Increased Chance: %1", chance);
			#endif
			return chance / 100;
		}
	}

	// This is called when the player shoots from player controller
	// Degrades per shot
	void Degrade(int playerId, notnull BaseWeaponComponent weapon)
	{
		m_fCurrentDurability -= m_fDegradePerShot;
		if (m_fCurrentDurability < 0)
			m_fCurrentDurability = 0;
		
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || !rplComp.IsOwner())
			return;
		
		Replication.BumpMe();
		
		#ifdef WORKBENCH
		PrintFormat("Durability: %1", m_fCurrentDurability);
		#endif
		if (m_fCurrentDurability <= 0) {
			PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (playerController) {
				IEntity ent = playerController.GetControlledEntity();
				if (!ent)
					return;
				
				SCR_CharacterDamageManagerComponent charDamManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));

				if (!charDamManager)
					return;
				
				// Particle effect?
				if (m_sExplosionEffect) {
					RplId entityId = Replication.FindId(this);
					if (entityId.IsValid()) {
						Rpc_Broadcast_CreateParticleEffect(entityId);
						Rpc(Rpc_Broadcast_CreateParticleEffect, entityId);
					}
				}
				
				if (m_bDamageOnExplode)
					charDamManager.AddParticularBleeding();
				
				charDamManager.ForceUnconsciousness(0);

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
				RandomGenerator rand = new RandomGenerator();
				float dice = rand.RandFloat01();
				if (dice <= JamChance()) {
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
}
