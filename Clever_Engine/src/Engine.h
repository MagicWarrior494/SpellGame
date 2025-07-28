#pragma once
#include "Event/EventController.h"
#include "Render/RenderingController.h"
#include "World/WorldController.h"

namespace Engine {

	class Engine 
	{
	public:
		void SetUp();
		void Terminate();

		//TODO: Add ticking, so calling Step is not required but all per frame updates get added to a 
		//      queue to be processed next(or subsequent) frame(s)
		void Step();
		void Test();
		void Test1();
	private:
		/*RenderingController renderingController;
		EventController eventController;*/
		WorldController worldController;
	};
}