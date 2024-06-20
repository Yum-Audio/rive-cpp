#include "rive/data_bind/context/context_value_string.hpp"
#include "rive/generated/core_registry.hpp"

using namespace rive;

DataBindContextValueString::DataBindContextValueString(ViewModelInstanceValue* value)
{
    m_Source = value;
}

void DataBindContextValueString::apply(Component* target, uint32_t propertyKey)
{
    CoreRegistry::setString(target,
                            propertyKey,
                            m_Source->as<ViewModelInstanceString>()->propertyValue());
}

void DataBindContextValueString::applyToSource(Component* target, uint32_t propertyKey)
{
    auto value = CoreRegistry::getString(target, propertyKey);
    if (m_Value != value)
    {
        m_Value = value;
        m_Source->as<ViewModelInstanceString>()->propertyValue(value);
    }
}