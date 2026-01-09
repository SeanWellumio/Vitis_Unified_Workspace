/* platform_i2c.c */
#include "platform_i2c.h"
#include "xiic.h"   /* or xiicps.h for PS I2C */
#include "xparameters.h"
#include <string.h>

static XIic AxiIicInst;
static int g_inited = 0;


#define AXI_IIC_DEVICE_ID           XPAR_AXI_IIC_0_DEVICE_ID

/* Example: initialize XIic (AXI IIC driver) */
int platform_i2c_init(uint32_t axi_iic_device_id)
{
    if (g_inited) return 0; /* idempotent */

    int Status;
    XIic_Config *CfgPtr;

    CfgPtr = XIic_LookupConfig(axi_iic_device_id);
    if (CfgPtr == NULL) {
        xil_printf("AXI IIC: LookupConfig failed\r\n");
        return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&AxiIicInst, CfgPtr, CfgPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("AXI IIC: CfgInitialize failed: %d\r\n", Status);
        return Status;
    }

    XIic_Reset(&AxiIicInst);
    XIic_Start(&AxiIicInst);
    XIic_IntrGlobalDisable(AxiIicInst.BaseAddress);

    xil_printf("AXI IIC init OK @ 0x%08x\r\n",
               (unsigned)AxiIicInst.BaseAddress);

    /* Set global inited */
    g_inited = 1;
    return XST_SUCCESS;
}

void platform_i2c_deinit(void)
{
    if (!g_inited) return;
    XIic_Stop(&AxiIicInst);
    /* deconfigure interrupts/gpios if needed */
    g_inited = 0;
}




#define MUX_ADDR 0x70

/* example mapping from logical bus id -> mux selection or nothing */
int platform_i2c_write_mux(uint8_t value) {
    if (!g_inited) return -1;

    int Sent;
    u8 buf = 1 << value;

    Sent = XIic_Send(AxiIicInst.BaseAddress,
                     MUX_ADDR,
                     &buf,
                     1,
                     XIIC_STOP);

    return (Sent == 1) ? XST_SUCCESS : XST_FAILURE;
}

/* example mapping from logical bus id -> mux selection or nothing */
int platform_i2c_read_mux(uint8_t *out) {
    if (!g_inited) return -1;

    int Recv;
    u8 buf;

    // Read 1 byte, with STOP at the end
    Recv = XIic_Recv(AxiIicInst.BaseAddress,
                     MUX_ADDR,
                     &buf,
                     1,
                     XIIC_STOP);
    // xil_printf("Recv: Recv=%d\r\n", Recv);
    if (Recv != 1) return XST_FAILURE;

    *out = buf;
    return XST_SUCCESS;
}


/* blocking write: compose [reg, data...] and send via XIic_Send */
int platform_i2c_write(uint8_t dev_addr, uint8_t reg, const uint8_t *data, uint8_t len)
{
    if (!g_inited) return -1;

    /* set mux for bus (prefer GPIO toggling here) */
    /* platform_set_mux_gpio(bus); */

    uint8_t txbuf[1 + 16]; /* adjust max len */
    if (len > 15) return -2; /* enlarge buffer if needed */
    txbuf[0] = reg;
    if (len) memcpy(&txbuf[1], data, len);

    int retries = 0;
    const int max_retries = 2;
    while (retries <= max_retries) {
        int rc = XIic_Send(AxiIicInst.BaseAddress, dev_addr, txbuf, 1 + len, XIIC_STOP);
        /* XIic_Send return semantics vary; check for success */
        if (rc >= 0) return 0;
        retries++;
    }
    return -3;
}

int platform_i2c_read(uint8_t addr7, uint8_t reg, uint16_t *out)
{
    if (!g_inited) return -1;

    int Sent, Recv;
    u8 buf[2];

    // Set pointer register, with repeated start (no STOP)
    Sent = XIic_Send(AxiIicInst.BaseAddress,
                     addr7,
                     &reg,
                     1,
                     XIIC_REPEATED_START);
    // xil_printf("Send: Sent=%d\r\n", Sent);
    if (Sent != 1) return XST_FAILURE;

    // Read 2 bytes, with STOP at the end
    Recv = XIic_Recv(AxiIicInst.BaseAddress,
                     addr7,
                     buf,
                     2,
                     XIIC_STOP);
    // xil_printf("Recv: Recv=%d\r\n", Recv);
    if (Recv != 2) return XST_FAILURE;

    *out = ((u16)buf[0] << 8) | buf[1];
    return XST_SUCCESS;
}