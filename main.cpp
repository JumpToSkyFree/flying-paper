#include <Application.hpp>
#include <peel/RefPtr.h>

int main(int argc, char *argv[]) {
  peel::RefPtr<FlyingPaper::Application::Application> app =
      FlyingPaper::Application::Application::create();
  return app->run(argc, argv);
}
