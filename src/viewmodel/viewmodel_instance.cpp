#include <sstream>
#include <iomanip>
#include <array>

#include "rive/viewmodel/viewmodel_instance.hpp"
#include "rive/viewmodel/viewmodel.hpp"
#include "rive/viewmodel/viewmodel_instance_value.hpp"
#include "rive/importers/viewmodel_importer.hpp"
#include "rive/core_context.hpp"

using namespace rive;

void ViewModelInstance::addValue(ViewModelInstanceValue* value)
{
    m_PropertyValues.push_back(value);
}

StatusCode ViewModelInstance::onAddedDirty(CoreContext* context)
{
    StatusCode result = Super::onAddedDirty(context);
    if (result != StatusCode::Ok)
    {
        return result;
    }
    auto coreObject = context->resolve(viewModelId());
    if (coreObject != nullptr && coreObject->is<ViewModel>())
    {
        m_ViewModel = static_cast<ViewModel*>(coreObject);
    }

    return StatusCode::Ok;
}

ViewModelInstanceValue* ViewModelInstance::propertyValue(const uint32_t id)
{
    for (auto value : m_PropertyValues)
    {
        if (value->viewModelPropertyId() == id)
        {
            return value;
        }
    }
    return nullptr;
}

ViewModelInstanceValue* ViewModelInstance::propertyValue(const std::string& name)
{
    auto viewModelProperty = viewModel()->property(name);
    if (viewModelProperty != nullptr)
    {
        for (auto value : m_PropertyValues)
        {
            if (value->viewModelProperty() == viewModelProperty)
            {
                return value;
            }
        }
    }
    return nullptr;
}

void ViewModelInstance::viewModel(ViewModel* value) { m_ViewModel = value; }

ViewModel* ViewModelInstance::viewModel() { return m_ViewModel; }

void ViewModelInstance::onComponentDirty(Component* component) {}

void ViewModelInstance::setAsRoot() { setRoot(this); }

void ViewModelInstance::setRoot(ViewModelInstance* value)
{
    for (auto propertyValue : m_PropertyValues)
    {
        propertyValue->setRoot(value);
    }
}

std::vector<ViewModelInstanceValue*> ViewModelInstance::propertyValues()
{
    return m_PropertyValues;
}

Core* ViewModelInstance::clone() const
{
    auto cloned = new ViewModelInstance();
    cloned->copy(*this);
    for (auto propertyValue : m_PropertyValues)
    {
        auto clonedValue = propertyValue->clone()->as<ViewModelInstanceValue>();
        cloned->addValue(clonedValue);
    }
    return cloned;
}

StatusCode ViewModelInstance::import(ImportStack& importStack)
{
    auto viewModelImporter = importStack.latest<ViewModelImporter>(ViewModel::typeKey);
    if (viewModelImporter == nullptr)
    {
        return StatusCode::MissingObject;
    }

    viewModelImporter->addInstance(this);
    return StatusCode::Ok;
}
