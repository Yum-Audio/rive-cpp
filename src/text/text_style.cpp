#include "rive/text/text_style.hpp"
#include "rive/text/text_style_axis.hpp"
#include "rive/renderer.hpp"
#include "rive/shapes/paint/shape_paint.hpp"
#include "rive/backboard.hpp"
#include "rive/importers/backboard_importer.hpp"
#include "rive/text/text.hpp"
#include "rive/artboard.hpp"
#include "rive/factory.hpp"

using namespace rive;

namespace rive
{
class TextVariationHelper : public Component
{
public:
    TextVariationHelper(TextStyle* style) : m_textStyle(style) {}
    TextStyle* style() const { return m_textStyle; }
    void buildDependencies() override
    {
        auto text = m_textStyle->parent();
        text->artboard()->addDependent(this);
        addDependent(text);
    }

    void update(ComponentDirt value) override { m_textStyle->updateVariableFont(); }

private:
    TextStyle* m_textStyle;
};
} // namespace rive

// satisfy unique_ptr
TextStyle::TextStyle() {}

void TextStyle::addVariation(TextStyleAxis* axis) { m_variations.push_back(axis); }

void TextStyle::onDirty(ComponentDirt dirt)
{
    if ((dirt & ComponentDirt::TextShape) == ComponentDirt::TextShape)
    {
        parent()->as<Text>()->markShapeDirty();
        if (m_variationHelper != nullptr)
        {
            m_variationHelper->addDirt(ComponentDirt::TextShape);
        }
    }
}

StatusCode TextStyle::onAddedClean(CoreContext* context)
{
    auto code = Super::onAddedClean(context);
    if (code != StatusCode::Ok)
    {
        return code;
    }
    // This ensures context propagates to variation helper too.
    if (!m_variations.empty())
    {
        m_variationHelper = rivestd::make_unique<TextVariationHelper>(this);
    }
    if (m_variationHelper != nullptr)
    {
        if ((code = m_variationHelper->onAddedDirty(context)) != StatusCode::Ok)
        {
            return code;
        }
        if ((code = m_variationHelper->onAddedClean(context)) != StatusCode::Ok)
        {
            return code;
        }
    }
    return StatusCode::Ok;
}

const rcp<Font> TextStyle::font() const
{
    if (m_variableFont != nullptr)
    {
        return m_variableFont;
    }
    return m_fontAsset == nullptr ? nullptr : m_fontAsset->font();
}

void TextStyle::updateVariableFont()
{
    rcp<Font> baseFont = m_fontAsset == nullptr ? nullptr : m_fontAsset->font();
    if (baseFont == nullptr)
    {
        // Not ready yet.
        return;
    }
    if (!m_variations.empty())
    {
        m_coords.clear();
        for (TextStyleAxis* axis : m_variations)
        {
            m_coords.push_back({axis->tag(), axis->axisValue()});
        }
        m_variableFont = baseFont->makeAtCoords(m_coords);
    }
    else
    {
        m_variableFont = nullptr;
    }
}

void TextStyle::buildDependencies()
{
    if (m_variationHelper != nullptr)
    {
        m_variationHelper->buildDependencies();
    }
    parent()->addDependent(this);
    Super::buildDependencies();
    auto factory = getArtboard()->factory();
    m_path = factory->makeEmptyRenderPath();
}

void TextStyle::rewindPath()
{
    m_path->rewind();
    m_hasContents = false;
    m_opacityPaths.clear();
}

bool TextStyle::addPath(const RawPath& rawPath, float opacity)
{
    bool hadContents = m_hasContents;
    m_hasContents = true;
    if (opacity == 1.0f)
    {
        rawPath.addTo(m_path.get());
    }
    else if (opacity > 0.0f)
    {
        auto itr = m_opacityPaths.find(opacity);
        RenderPath* renderPath = nullptr;
        if (itr != m_opacityPaths.end())
        {
            renderPath = itr->second.get();
        }
        else
        {
            auto factory = getArtboard()->factory();
            auto erp = factory->makeEmptyRenderPath();
            renderPath = erp.get();
            m_opacityPaths[opacity] = std::move(erp);
        }
        rawPath.addTo(renderPath);
    }

    return !hadContents;
}

void TextStyle::draw(Renderer* renderer)
{
    auto path = m_path.get();
    for (auto shapePaint : m_ShapePaints)
    {
        if (!shapePaint->isVisible())
        {
            continue;
        }
        shapePaint->draw(renderer, path);

        if (m_paintPool.size() < m_opacityPaths.size())
        {
            m_paintPool.reserve(m_opacityPaths.size());
            Factory* factory = artboard()->factory();
            while (m_paintPool.size() < m_opacityPaths.size())
            {
                m_paintPool.emplace_back(factory->makeRenderPaint());
            }
        }

        uint32_t paintIndex = 0;
        for (auto itr = m_opacityPaths.begin(); itr != m_opacityPaths.end(); itr++)
        {
            RenderPaint* renderPaint = m_paintPool[paintIndex++].get();
            shapePaint->applyTo(renderPaint, itr->first);
            shapePaint->draw(renderer, itr->second.get(), renderPaint);
        }
    }
}

void TextStyle::assets(const std::vector<FileAsset*>& assets)
{
    if ((size_t)fontAssetId() >= assets.size())
    {
        return;
    }
    auto asset = assets[fontAssetId()];
    if (asset->is<FontAsset>())
    {
        m_fontAsset = asset->as<FontAsset>();
    }
}

StatusCode TextStyle::import(ImportStack& importStack)
{
    auto result = registerReferencer(importStack);
    if (result != StatusCode::Ok)
    {
        return result;
    }
    return Super::import(importStack);
}

void TextStyle::fontSizeChanged() { parent()->as<Text>()->markShapeDirty(); }

Core* TextStyle::clone() const
{
    TextStyle* twin = TextStyleBase::clone()->as<TextStyle>();
    twin->m_fontAsset = m_fontAsset;
    return twin;
}