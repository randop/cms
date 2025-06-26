#!/usr/bin/env bash

set -euo pipefail

MAX_ATTEMPTS=5
SLEEP_INTERVAL=30
ATTEMPT=1
DEPLOY_ID=0

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
  elif [ "$STATUS" = "failed" ] || [ "$STATUS" = "canceled" ]; then
    echo "Deploy failed or was canceled."
    exit 1
  fi

  echo "Deploy is still in progress ($STATUS). Waiting $SLEEP_INTERVAL seconds..."
  sleep $SLEEP_INTERVAL
  ATTEMPT=$((ATTEMPT + 1))
done

echo "Timed out waiting for deploy to complete."
exit 1