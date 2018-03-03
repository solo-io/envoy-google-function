# Google Cloud Functions

## Overview

Google Cloud Functions is an HTTP filter which enables Envoy to integrate with
[Google Cloud Functions](https://cloud.google.com/functions/).

Once a request routed for Google Cloud Functions enters the service mesh, the Google Cloud
Functions Envoy filter forwards it to the designated Cloud Function in the appropriate region.
