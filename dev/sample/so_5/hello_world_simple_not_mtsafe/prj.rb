require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'so_5/prj.rb'
	target 'sample.so_5.hello_world_simple_not_mtsafe'

	cpp_source 'main.cpp'
}
