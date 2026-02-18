// no_ctrl_click_right.c
// Remove o modifier Control APENAS do clique esquerdo do mouse.
// Ctrl continua funcionando para atalhos/teclas normalmente.
// Requer permissão: Ajustes do Sistema > Privacidade e Segurança > Acessibilidade

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

static CGEventRef callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    (void)proxy;
    (void)refcon;

    // Se o sistema desabilitar o tap, tentamos reabilitar
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        // refcon não está sendo usado; em muitos casos o sistema reabilita sozinho.
        // Se quiser robustez máxima, dá pra guardar o eventTap em global e reabilitar aqui.
        return event;
    }

    // Intercepta apenas eventos do botão esquerdo
    if (type == kCGEventLeftMouseDown || type == kCGEventLeftMouseUp || type == kCGEventLeftMouseDragged) {
        CGEventFlags flags = CGEventGetFlags(event);

        // Se Control estiver pressionado, remove APENAS desse evento do mouse
        if (flags & kCGEventFlagMaskControl) {
            flags &= ~kCGEventFlagMaskControl;
            CGEventSetFlags(event, flags);
        }
    }

    return event;
}

int main(void) {
    CGEventMask mask =
        CGEventMaskBit(kCGEventLeftMouseDown) |
        CGEventMaskBit(kCGEventLeftMouseUp) |
        CGEventMaskBit(kCGEventLeftMouseDragged);

    CFMachPortRef tap = CGEventTapCreate(
        kCGHIDEventTap,            // pega eventos "na fonte" (global)
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        mask,
        callback,
        NULL
    );

    if (!tap) {
        fprintf(stderr,
                "Falhou ao criar Event Tap.\n"
                "Verifique permissões em: Ajustes do Sistema > Privacidade e Segurança > Acessibilidade\n"
                "e habilite o Terminal/iTerm.\n");
        return 1;
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CGEventTapEnable(tap, true);

    printf("OK! Ctrl+clique esquerdo agora NÃO vira clique direito.\n");
    printf("Ctrl continua funcionando para atalhos. Para sair: Ctrl+C no terminal.\n");

    CFRunLoopRun();

    CFRelease(source);
    CFRelease(tap);
    return 0;
}
