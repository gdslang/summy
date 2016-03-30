/*
 * summary_dstack_complex_noerror.cpp
 *
 *  Created on: Mar 30, 2016
 *      Author: Julian Kranz
 */

/*
 * This file contains complex test that should only not trigger an error.
 */

#include <summy/test/analysis/domains/common.h>
#include <gtest/gtest.h>

class summary_complex_noerror_test : public ::testing::Test {
protected:
  summary_complex_noerror_test() {}

  virtual ~summary_complex_noerror_test() {}

  virtual void SetUp() {}

  virtual void TearDown() {}
};

TEST_F(summary_complex_noerror_test, TestIssue43) {
  // test_malloc2.c
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state(ar, "\n\
#include <string.h>\n\
#include <stdio.h>\n\
\n\
/* generated declarations for records with fixed fields */\n\
typedef struct {\n\
  void* payload;\n\
} struct1_t;\n\
\n\
\n\
struct con_struct1 {\n\
  int tag;\n\
  struct1_t payload;\n\
};\n\
\n\
/* mkN */\n\
int mkN(void* s,void* v,void* l) {\n\
  if (*((int*) l)) {\n\
        return printf(s,v,1);\n\
  } else {\n\
    struct1_t x;\n\
    x = ((struct con_struct1*) l)->payload;\n\
  }\n\
}\n\
\n\
int main(int argc, char **argv) {\n\
  return mkN(argv, argv[0], argv[1]);\n\
}",
    C, false, 1));
}

TEST_F(summary_complex_noerror_test, TestIssue44) {
  // test_malloc2.c
  _analysis_result ar;
  ASSERT_NO_FATAL_FAILURE(state(ar, "\n\
#include <string.h>\n\
#include <stdio.h>\n\
\n\
typedef struct {\n\
  int rope_size;\n\
  char* rope_string;\n\
} struct1_t;\n\
\n\
\n\
struct con_struct1 {\n\
  int tag;\n\
  struct1_t payload;\n\
};\n\
\n\
static void rope_print(void *r) {\n\
  switch (((struct con_struct1*) r)->tag) {\n\
    case 3: {\n\
      fputs(NULL, NULL);\n\
    }; break;\n\
    case 4: {\n\
      struct1_t n;\n\
      n = ((struct con_struct1*) r)->payload;\n\
    }; break;\n\
    default: {\n\
      longjmp(NULL,0);\n\
    }; break;\n\
  };\n\
}\n\
\n\
int main(int argc, char **argv) {\n\
  rope_print(argv);\n\
  return 0;\n\
}",
    C, false, 1));
}
