#include "./lib.h"
#include "../lib/typex.h"

int main(void)
{
	START_CASE("00-noop.c");
	io_string_t in;

	int err = file_into_memory("./test/examples/00-noop.c", &in);
	ASSERT_EQ(err, 0, "00-noop.c", "should be able to read file into memory");

	owned_str_t str;
	err = typex_preprocess(in.buff, in.len, &str);
	ASSERT_EQ(err, 0, "00-noop.c", "should be able to preprocess empty file");

	free(in.buff);
	free(str.buff);

	SUCCESS("00-noop.c");
}

