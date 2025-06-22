#!/bin/bash
set -e

echo "Checking if Coverity scan should run..."

if [ "$APPVEYOR_BUILD_WORKER_IMAGE" = "Ubuntu2204" ] && \
   [ "$PLATFORM" = "x64" ] && \
   [ "$CONFIGURATION" = "Debug" ]; then

  if [ -z "$COVERITY_SCAN_TOKEN" ]; then
    echo "ERROR: COVERITY_SCAN_TOKEN is not set."
    exit 1
  fi

  rm -rf build

  echo "Running Coverity scan on Ubuntu2204 x64 Debug..."
  echo "APPVEYOR_BUILD_VERSION=$APPVEYOR_BUILD_VERSION"

  # Download Coverity tool
  COV_DIR="$HOME/cov-analysis"
  COV_TOOL_URL="https://scan.coverity.com/download/linux64"
  COV_ARCHIVE="coverity_tool.tgz"

  curl -s -L \
     --form project=ilqvya/random \
     --form token=$COVERITY_SCAN_TOKEN \
     https://scan.coverity.com/download/linux64 \
     -o $COV_ARCHIVE

  mkdir -p "$COV_DIR"
  tar -xzf "$COV_ARCHIVE" -C "$COV_DIR" --strip-components=1

  # Build with Coverity
  "$COV_DIR/bin/cov-build" --dir cov-int ./appveyor_build_lin.sh

  # Ignore test and usr directory
  echo "Ignoring test and usr dir: "
  ls -la cov-int/
  find cov-int/emit -type f -name '*.tu' | grep -E '/(usr|test)/' | xargs rm -v
  
  # Package and upload
  tar czf coverity.tgz cov-int

  curl --silent --form token=$COVERITY_SCAN_TOKEN \
                --form email=ilqvya@icloud.com \
                --form file=@coverity.tgz \
                --form version="$APPVEYOR_BUILD_VERSION" \
                --form description="AppVeyor CI build" \
                https://scan.coverity.com/builds?project=ilqvya%2Frandom
else
  echo "Skipping Coverity scan for this job: OS=$APPVEYOR_OS_NAME, PLATFORM=$PLATFORM, CONFIGURATION=$CONFIGURATION"
fi
