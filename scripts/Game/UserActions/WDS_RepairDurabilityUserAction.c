class WDS_RepairDurabilityUserAction : ScriptedUserAction
{
	protected WDS_DurabilityComponent m_DurabilityComponent;
	protected IEntity m_OwnerEntity;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DurabilityComponent = WDS_DurabilityComponent.Cast(pOwnerEntity.FindComponent(WDS_DurabilityComponent));
		m_OwnerEntity = pOwnerEntity;
		
		super.Init(pOwnerEntity, pManagerComponent);
	}
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_DurabilityComponent)
			return;
		
		RplComponent rplComp = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		if (!rplComp || !rplComp.IsOwner())
			return;
		
		WDS_RepairItemPredictate predicate();
		SCR_InventoryStorageManagerComponent manager = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!manager)
			return;
		
		IEntity item = manager.FindItem(predicate, EStoragePurpose.PURPOSE_ANY);
		if (!item)
			return;
		
		float amount = 15.0;
		WDS_RepairItemComponent repairItem = WDS_RepairItemComponent.Cast(item.FindComponent(WDS_RepairItemComponent));
		if (repairItem)
			amount = repairItem.GetRestoreAmount();
		
		if (manager.TryDeleteItem(item)) {
			float max = m_DurabilityComponent.GetMaxDurability();
			float current = m_DurabilityComponent.GetCurrentDurability();
			
			current += amount;
			m_DurabilityComponent.SetDurability(current);
		}
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!user)
			return false;
		
		// Check for specific item in inventory
		WDS_RepairItemPredictate predicate();
		SCR_InventoryStorageManagerComponent manager = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!manager)
			return false;
		
		// If no item in storage with component say reason
		if (!manager.FindItem(predicate, EStoragePurpose.PURPOSE_ANY)) {
			SetCannotPerformReason("No Repair Item");
			return false;
		}
		
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		if (!m_OwnerEntity)
			return false;
		
		CharacterControllerComponent charController =  character.GetCharacterController();
		if (!charController)
			return false;
		
		if (!m_DurabilityComponent)
			return false;
		
		if (charController.GetInspectEntity() != m_OwnerEntity)
			return false;
		
		return true;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		float maxDurability = m_DurabilityComponent.GetMaxDurability();
		float durability = m_DurabilityComponent.GetCurrentDurability();
		
		int states = maxDurability / 4;
		
		string stateName = string.Empty;
		if (durability <= states)
			stateName = "Unstable";
		else if (durability <= 2 * states)
			stateName = "Poor";
		else if (durability <= 3 * states)
			stateName = "Good";
		else
			stateName = "Mint";
		
		outName = string.Format("Repair [COND: %1]", stateName);
		return true;
	}
	
	override bool HasLocalEffectOnlyScript()
	{ 
		return false;
	};
}
