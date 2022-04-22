@echo off
setlocal ENABLEDELAYEDEXPANSION

if "%1" == "" (
  GOTO usage
)
if "%1" == "cosmos" (
  set params=%*
  call set params=%%params:*%1=%%
  REM Start (and remove when done --rm) the cosmos-base container with the current working directory
  REM mapped as volume (-v) /cosmos/local and container working directory (-w) also set to /cosmos/local.
  REM This allows tools running in the container to have a consistent path to the current working directory.
  REM Run the command "ruby /cosmos/bin/cosmos" with all parameters ignoring the first.
  docker run --rm -v %cd%:/cosmos/local -w /cosmos/local ballaerospace/cosmosc2-base ruby /cosmos/bin/cosmos !params!
  GOTO :EOF
)
if "%1" == "restart" (
  GOTO restart
)
if "%1" == "start" (
  GOTO startup
)
if "%1" == "stop" (
  GOTO stop
)
if "%1" == "cleanup" (
  GOTO cleanup
)
if "%1" == "build" (
  GOTO build
)
if "%1" == "run" (
  GOTO run
)
if "%1" == "dev" (
  GOTO dev
)
if "%1" == "dind" (
  GOTO dind
)
if "%1" == "deploy" (
  GOTO deploy
)
if "%1" == "test" (
  GOTO test
)
if "%1" == "util" (
  GOTO util
)

GOTO usage

:startup
  CALL cosmos-control build
  docker-compose -f compose.yaml up -d
  @echo off
GOTO :EOF

:restart
  docker-compose -f compose.yaml restart
  @echo off
GOTO :EOF

:stop
  docker-compose -f compose.yaml down
  @echo off
GOTO :EOF

:cleanup
  docker-compose -f compose.yaml down -v
  @echo off
GOTO :EOF

:build
  CALL scripts\windows\cosmos_setup
  docker-compose -f compose.yaml -f compose-build.yaml build cosmos-ruby || exit /b
  docker-compose -f compose.yaml -f compose-build.yaml build cosmos-base || exit /b
  docker-compose -f compose.yaml -f compose-build.yaml build cosmos-node || exit /b
  docker-compose -f compose.yaml -f compose-build.yaml build || exit /b
  @echo off
GOTO :EOF

:run
  docker-compose -f compose.yaml up -d
  @echo off
GOTO :EOF

:dev
  docker-compose -f compose.yaml -f compose-dev.yaml up -d
  @echo off
GOTO :EOF

:dind
  docker build -t cosmos-build .
  docker run --rm -ti -v /var/run/docker.sock:/var/run/docker.sock cosmos-build
  @echo off
GOTO :EOF

:deploy
  REM Send the remaining arguments to cosmos_deploy
  set args=%*
  call set args=%%args:*%1=%%
  CALL scripts\windows\cosmos_deploy %args%
  @echo off
GOTO :EOF

:test
  REM Building COSMOS
  CALL scripts\windows\cosmos_setup
  docker-compose -f compose.yaml -f compose-build.yaml build
  set args=%*
  call set args=%%args:*%1=%%
  REM Running tests
  CALL scripts\windows\cosmos_test %args%
  @echo off
GOTO :EOF

:util
  REM Send the remaining arguments to cosmos_util
  set args=%*
  call set args=%%args:*%1=%%
  CALL scripts\windows\cosmos_util %args%
  @echo off
GOTO :EOF

:usage
  @echo Usage: %0 [start, stop, cleanup, build, run, deploy, util] 1>&2
  @echo *  cosmos: run a cosmos command ('cosmos help' for more info) 1>&2
  @echo *  start: run the docker containers for cosmos 1>&2
  @echo *  stop: stop the running docker containers for cosmos 1>&2
  @echo *  restart: stop and run the minimal docker containers for cosmos 1>&2
  @echo *  cleanup: cleanup network and volumes for cosmos 1>&2
  @echo *  build: build the containers for cosmos 1>&2
  @echo *  run: run the prebuilt containers for cosmos 1>&2
  @echo *  dev: run cosmos in dev mode 1>&2
  @echo *  dind: build and run the docker development container (cosmos-build) 1>&2
  @echo *  deploy: deploy the containers to localhost repository 1>&2
  @echo *    repository: hostname of the docker repository 1>&2
  @echo *  test: test COSMOS 1>&2
  @echo *    rspec: run tests against Ruby code 1>&2
  @echo *    cypress: run end-to-end tests 1>&2
  @echo *  util: various helper commands 1>&2
  @echo *    encode: encode a string to base64 1>&2
  @echo *    hash: hash a string using SHA-256 1>&2

@echo on
