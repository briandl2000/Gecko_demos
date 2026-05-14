#include "App.h"

#include <cstdio>

int main()
{
  App app;
  const int rc = app.Run();
  ::std::printf(rc == 0 ? "Graphics example finished.\n" : "Graphics example exited with code %d\n", rc);
  return rc;
}
