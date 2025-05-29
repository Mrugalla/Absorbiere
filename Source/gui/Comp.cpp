#include "Comp.h"

namespace gui
{
	Comp::Comp(Utils& u, const String& uID) :
		utils(u),
		layout(),
		tooltip(""),
		members(),
		callbacks(),
		scale(1.f),
		shearX(0.f),
		shearY(0.f)
	{
		setName(uID);
		const auto& props = utils.getProps();
		const auto transformString = props.getValue("cmp_" + getName(), "");
		setTransformFromString(transformString);

		setMouseCursor(makeCursor());

		addEvt([this](const evt::Type type, const void* stuff)
		{
			if(type == evt::Type::ThemeUpdated)
			{
				const auto cID = *static_cast<const CID*>(stuff);
				if (cID == CID::Interact)
					setMouseCursor(makeCursor());
			}
		});
	}

	Comp::~Comp()
	{
		deregisterCallbacks();
		
		const auto transformString = getTransformString();
		const auto name = getName();
		auto& props = utils.getProps();
		props.setValue("cmp_" + name, transformString);
	}

	void Comp::setLocked(bool e)
	{
		setAlpha(e ? LockAlpha : 1.f);
	}

	void Comp::deregisterCallbacks()
	{
		for (auto& cb : callbacks)
			utils.remove(&cb);
	}

	void Comp::add(const Callback& callback)
	{
		deregisterCallbacks();
		callbacks.push_back(callback);
		registerCallbacks();
	}

	void Comp::registerCallbacks()
	{
		for (auto& cb : callbacks)
			utils.add(&cb);
	}

	void Comp::initLayout(const String& xL, const String& yL)
	{
		layout.initFromStrings(xL, yL);
	}

	void Comp::addEvt(const evt::Evt& evt)
	{
		members.push_back({ utils.eventSystem, evt });
	}

	void Comp::notify(const evt::Type type, const void* stuff)
	{
		utils.eventSystem.notify(type, stuff);
	}

	void Comp::mouseEnter(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::TooltipUpdated, &tooltip);
	}

	void Comp::mouseUp(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::ClickedEmpty, this);
	}

	void Comp::setTooltip(const String& t)
	{
		tooltip = t;
		if(tooltip.isNotEmpty())
			setInterceptsMouseClicks(true, true);
	}
}
