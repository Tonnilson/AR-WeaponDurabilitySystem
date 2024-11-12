[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class WDS_DurabilityPlayerComponentClass : ScriptComponentClass
{
}

class WDS_DurabilityPlayerComponent : ScriptComponent
{
	protected SCR_CharacterControllerComponent GetCharacterController(IEntity from)
	{
		if (!from)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(from);
		if (!character)
			return SCR_CharacterControllerComponent.Cast(from.FindComponent(SCR_CharacterControllerComponent));

		return SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	}
	
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (from)
			UnregisterEvents(from);
		
		if (to)
			RegisterEvents(to);
	}
	
	protected void RegisterEvents(IEntity ent)
	{
		if (!ent)
			return;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager) {
			eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, OnWeaponFired);
		}
	}
	
	protected void UnregisterEvents(IEntity ent)
	{
		if (!ent)
			return;
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager) {
			eventHandlerManager.RemoveScriptHandler("OnProjectileShot", this, OnWeaponFired);
		}
	}
	
	protected void OnWeaponFired(int playerId, BaseWeaponComponent weapon, IEntity entity)
	{
		WDS_DurabilityComponent duraComp = WDS_DurabilityComponent.Cast(weapon.GetOwner().FindComponent(WDS_DurabilityComponent));
		if (duraComp) {
			duraComp.Degrade(playerId, weapon);
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
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
			return;
		
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// remove if unused
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		// remove if unused
	}
	/*
	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		SetEventMask(owner, EntityEvent.FRAME);
	}
 
	override void EOnDeactivate(IEntity owner)
	{
		 super.EOnDeactivate(owner);
		ClearEventMask(owner, EntityEvent.FRAME);
	}
	*/
}
