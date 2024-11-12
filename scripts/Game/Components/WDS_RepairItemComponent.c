[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class WDS_RepairItemComponentClass : ScriptComponentClass
{
}

class WDS_RepairItemComponent : ScriptComponent
{
	[Attribute(defvalue: "25.00", category: "Repair Setting", desc: "Condition restore amount")]
	protected float m_fRestoreAmount;
	
	float GetRestoreAmount()
	{
		return m_fRestoreAmount;
	}
}

class WDS_RepairItemPredictate : InventorySearchPredicate
{
	void WDS_RepairItemPredictate()
	{
		QueryComponentTypes.Insert(WDS_RepairItemComponent);
	}
	
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return WDS_RepairItemComponent.Cast(queriedComponents[0]);
	}
}
