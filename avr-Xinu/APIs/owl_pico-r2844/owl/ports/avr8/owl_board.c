#include <board.h>
#include <compiler.h>
#include <sysclk.h>
#include <pmic.h>

#include <owl/drivers/owl_board.h>

int owlboard_init(void)
{
	pmic_init();
	sysclk_init();
        board_init();
        sysclk_enable_module(SYSCLK_PORT_C, PR_USART0_bm);
        sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm);
	cpu_irq_enable();
        return 0;
}

