#include "ui.h"
#include "api.h"
#include "parser.h"

int main(void) {
  api_intialize();
  page_t *page = api_get_page(HOME);

  return 0;
}
