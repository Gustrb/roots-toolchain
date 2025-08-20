#include "./lib.h"
#include "../lib/typex.h"

#include <string.h>

int main(void)
{
	START_CASE("01-define.c");
	io_string_t in;

	int err = file_into_memory("./test/examples/01-define.c", &in);
	ASSERT_EQ(err, 0, "01-define.c", "should be able to read file into memory");

	owned_str_t str;
	err = typex_preprocess(in.buff, in.len, &str);
	ASSERT_EQ(err, 0, "01-define.c", "should be able to preprocess empty file");

	const char *expected = "int main(void)\n{\treturn 0;\n}\n";
	size_t expected_len = strlen(expected);
	ASSERT_EQ(str.len >= expected_len, 0, "01-define.c", "the output should be bigger or equal than expected");
	
	free(in.buff);
	free(str.buff);

	SUCCESS("01-define.c");
}

