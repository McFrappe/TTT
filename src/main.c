#include "ui.h"
#include "api.h"
#include "parser.h"
#include <stdlib.h>

int main(void) {
  api_intialize();
  page_t *home = api_get_page(HOME);
  page_t *news = api_get_page(NEWS);
  api_destroy();
  
  free(home);
  free(news);

  return 0;
}
