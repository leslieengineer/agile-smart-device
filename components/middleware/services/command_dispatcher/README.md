# Command Dispatcher

Bounded command parsing, authorization, idempotency, handler routing, and response delivery. It owns no product commands and has no transport/vendor dependency. Products register handlers; duplicate command IDs replay cached status without re-executing the handler.
