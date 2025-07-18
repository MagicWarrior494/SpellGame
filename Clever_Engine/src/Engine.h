#pragma once
#include "Event/EventController.h"
#include "Render/RenderingController.h"
#include "World/WorldController.h"

namespace Engine {

	class Engine 
	{
	public:
		EngineData SetUp();
		void Terminate();

		//TODO: Add ticking, so calling Step is not required but all per frame updates get added to a 
		//      queue to be processed next(or subsequent) frame(s)
		FrameData Step();

	private:
		RenderingController renderingController;
		EventController eventController;
		WorldController worldController;
	};
}