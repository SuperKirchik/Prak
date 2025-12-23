#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

int main() {
    printf("=== ╨в╨╡╤Б╤В ╨╗╨╡╨║╤Б╨╕╤З╨╡╤Б╨║╨╛╨│╨╛ ╨░╨╜╨░╨╗╨╕╨╖╨░╤В╨╛╤А╨░ Shell ===\n");
    printf("╨Т╨▓╨╛╨┤╨╕ ╨║╨╛╨╝╨░╨╜╨┤╤Л, ╨┐╤А╨╛╨▓╨╡╤А╨╕╨╝ ╤В╨╛╨║╨╡╨╜╨╕╨╖╨░╤Ж╨╕╤О\n");
    printf("Ctrl+D ╨╕╨╗╨╕ 'exit' ╨┤╨╗╤П ╨▓╤Л╤Е╨╛╨┤╨░\n\n");
    
    char line[1024];
    
    while (1) {
        printf("myshell> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n╨Т╤Л╤Е╨╛╨┤\n");
            break;
        }
        
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "exit") == 0) {
            break;
        }
        
        if (strlen(line) == 0) {
            continue;
        }
        
        token_list tokens;
        
        printf("\n╨Ъ╨╛╨╝╨░╨╜╨┤╨░: '%s'\n", line);
        
        if (tokenize_input(line, &tokens) != 0) {
            printf("╨Ю╤И╨╕╨▒╨║╨░ ╤В╨╛╨║╨╡╨╜╨╕╨╖╨░╤Ж╨╕╨╕!\n");
            continue;
        }
        
        printf("╨в╨╛╨║╨╡╨╜╤Л ╨┐╨╛╤Б╨╗╨╡ ╤А╨░╨╖╨▒╨╛╤А╨░:\n");
        print_tokens(&tokens);
        
        expand_environment_vars(&tokens);
        
        printf("╨в╨╛╨║╨╡╨╜╤Л ╨┐╨╛╤Б╨╗╨╡ ╨╖╨░╨╝╨╡╨╜╤Л ╨┐╨╡╤А╨╡╨╝╨╡╨╜╨╜╤Л╤Е:\n");
        print_tokens(&tokens);
        
        clear_token_list(&tokens);
        
    
    }
    
    return 0;
}
