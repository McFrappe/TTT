#include "ui.h"
#include "api.h"
#include "parser.h"

int main(void) {
  api_intialize();
  api_get_page(HOME);
  api_get_page(NEWS);
  api_destroy();

  return 0;
}
