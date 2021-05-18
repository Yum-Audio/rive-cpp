#ifndef _RIVE_BLEND_STATE_HPP_
#define _RIVE_BLEND_STATE_HPP_
#include "generated/animation/blend_state_base.hpp"
#include <stdio.h>
#include <vector>

namespace rive
{
	class BlendAnimation;
	class LayerStateImporter;
	class BlendState : public BlendStateBase
	{
		friend class LayerStateImporter;

	private:
		std::vector<BlendAnimation*> m_Animations;
		void addAnimation(BlendAnimation* animation);

	public:
#ifdef TESTING
		size_t animationCount() { return m_Animations.size(); }
		BlendAnimation* animation(size_t index) { return m_Animations[index]; }
#endif
	};
} // namespace rive

#endif