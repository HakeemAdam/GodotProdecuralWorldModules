
#include "register_types.h"
#include "core/object/class_db.h"
#include "modules/procedural_world/TestExample.h"
#include "modules/procedural_world/landscape/Landscape.h"

void initialize_procedural_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
	ClassDB::register_class<TestExample>();
	ClassDB::register_class<Landscape>();

	//GDREGISTER_CLASS(Landscape);
}

void uninitialize_procedural_world_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

