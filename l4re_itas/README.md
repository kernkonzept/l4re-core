# L4Re ITAS – The L4Re In-Task Service

L4Re applications usually contain an additional thread running L4Re ITAS, the
*L4Re In-Task Service*. It is the first thread running inside the application
task and serves the following purposes:

- Loading the application binary,
- starting the loaded application,
- becoming the application’s default pager and exception handler.

The L4Re ITAS thread itself is paged by an external region map server, typically
Moe.
