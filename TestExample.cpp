#include "TestExample.h"
#include "core/object/class_db.h"
#include "core/string/print_string.h"

void TestExample::_bind_methods(){
	ClassDB::bind_method(D_METHOD("print_msg"), &TestExample::print_msg);
}

void TestExample::print_msg() const {
	print_line("Hello Godot Module");
}
