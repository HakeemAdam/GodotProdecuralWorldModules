#pragma once

#include "core/object/object.h"
#include "core/object/ref_counted.h"
class TestExample: public RefCounted{
	GDCLASS(TestExample, RefCounted);

	protected:
		static void _bind_methods();

	public:
		TestExample()= default;
		~TestExample() override = default;

		void print_msg() const;
};
