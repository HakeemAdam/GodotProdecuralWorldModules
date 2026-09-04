
#include "register_types.h"
#include "modules/procedural_world/TestExample.h"

void initialize_procedural_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
	//GDREGISTER_CLASS(TestExample);
}

void uninitialize_procedural_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

