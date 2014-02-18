A simple INI parser based linked list.

Usage


    struct ini_context *ctx = NULL;
    
    ctx = ini_load("test.ini");
    if (!ctx) return;
    printf("string get %s\n", ini\_get\_string(ctx, "section\_name", "element\_name", "default_value"));
    ini_unload(ctx);

