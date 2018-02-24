#!/bin/bash
#

set -e

# # create function if doesnt exist
# aws lambda create-function --function-name captialize --runtime nodejs 
# invoke
# aws lambda invoke --function-name uppercase --payload '"abc"' /dev/stdout

./create_config.sh || ./e2e/create_config.sh 

ENVOY=${ENVOY:-envoy}

$ENVOY -c ./envoy.yaml --log-level debug & 
sleep 5

curl localhost:10000/googs --data '{"message": "Hello!"}' --request POST -H"content-type: application/json"|grep abc

echo PASS