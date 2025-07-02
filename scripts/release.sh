#!/usr/bin/env bash
# -*- mode: shell-script; indent-tabs-mode: nil; sh-basic-offset: 4; -*-
# ex: ts=8 sw=4 sts=4 et filetype=sh
#
#  release.sh
#
#  Copyright © 2010 — 2025 Randolph Ledesma
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -euo pipefail

RETRY=1
MAX_RETRIES=3
MAX_ATTEMPTS=5
SLEEP_INTERVAL=15
DEPLOY_ID=0

while [ $RETRY -le $MAX_RETRIES ]; do
  ATTEMPT=1
  echo "Deploying (Attempt $RETRY/$MAX_RETRIES)..."
  DEPLOY_RESPONSE=$(render deploys create ${RENDER_SERVICE_ID} --output json --confirm)
  echo "$DEPLOY_RESPONSE"
  DEPLOY_ID=$(echo "$DEPLOY_RESPONSE" | jq -r '.id')

  while [ $ATTEMPT -le $MAX_ATTEMPTS ]; do
    echo "Checking deploy status (Attempt $ATTEMPT/$MAX_ATTEMPTS)..."
    STATUS_RESPONSE=$(curl -s -H "Authorization: Bearer $RENDER_API_KEY" \
      "https://api.render.com/v1/services/$RENDER_SERVICE_ID/deploys/$DEPLOY_ID")
    STATUS=$(echo "$STATUS_RESPONSE" | jq -r '.status')

    echo "Deploy status: $STATUS"

    if [ "$STATUS" = "live" ]; then
      echo "Deploy successful!"
      exit 0
    elif [ "$STATUS" = "update_failed" ] || [ "$STATUS" = "failed" ] || [ "$STATUS" = "canceled" ]; then
      echo "Deploy failed or was canceled."
      break
    fi

    echo "Deploy is still in progress ($STATUS). Waiting $SLEEP_INTERVAL seconds..."
    sleep $SLEEP_INTERVAL
    ATTEMPT=$((ATTEMPT + 1))
  done

  echo "Deploy is still in progress ($STATUS). Waiting $SLEEP_INTERVAL seconds..."
  sleep $SLEEP_INTERVAL
  RETRY=$((RETRY + 1))
done

echo "Timed out waiting for deploy to complete."
exit 1