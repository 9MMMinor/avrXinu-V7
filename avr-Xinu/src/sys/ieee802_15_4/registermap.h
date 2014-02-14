//
//  rfr2_registermap.h
//  radio-15-4
//
//  Created by Michael M Minor on 10/30/13.
//
//

#ifndef radio_15_4_rfr2_registermap_h
#define radio_15_4_rfr2_registermap_h

/*TRAC_STATUS*/
enum trac_status	{
	SUCCESS=0,					/* (RX_AACK, TX_ARET) */
	SUCCESS_DATA_PENDING=1,		/* (TX_ARET) */
	SUCCESS_WAIT_FOR_ACK=2,		/* (RX_AACK) */
	CHANNEL_ACCESS_FAILURE=3,	/* (TX_ARET) */
	NO_ACK=5,					/* (TX_ARET) */
	INVALID=7,					/* (RX_AACK, TX_ARET) */
};
/*TRX_CMD*/
enum trx_cmd	{
	CMD_NOP=0x00,
	CMD_TX_START=0x02,
	CMD_FORCE_TRX_OFF=0x03,
	CMD_FORCE_PLL_ON=0x04,
	CMD_RX_ON=0x06,
	CMD_TRX_OFF=0x08,
	CMD_PLL_ON=0x09,			/* (TX_ON) */
	CMD_RX_AACK_ON=0x16,
	CMD_TX_ARET_ON=0x19,
	CMD_RESET=0x40,				/* non-register commands start at 0x40 */
	CMD_SLEEP=0x41,
};
/*TRX_STATUS*/
enum trx_status	{
	STATUS_BUSY_RX=0X01,
	STATUS_BUSY_TX=0X02,
	STATUS_RX_ON=0X06,
	STATUS_TRX_OFF=0X08,
	STATUS_PLL_ON=0X09,
	STATUS_SLEEP=0X0F,
	STATUS_BUSY_RX_AACK=0X11,
	STATUS_BUSY_TX_ARET=0X12,
	STATUS_RX_AACK_ON=0X16,
	STATUS_TX_ARET_ON=0X19,
	STATUS_STATE_TRANSITION_IN_PROGRESS=0X1F
};
/* SET_STATE */
enum set_state	{
	STATE_RX_ON=0X06,
	STATE_TRX_OFF=0X08,
	STATE_PLL_ON=0X09,
	STATE_SLEEP=0X0F,
	STATE_RX_AACK_ON=0X16,
	STATE_TX_ARET_ON=0X19,
	STATE_RESET=0x40,				/* non-register commands start at 0x40 */
};


/* The following construct is not portable, but hey, Phy Registers aren't anyway */
/* Just remember the order (BYTE_ORDER == LITTLE_ENDIAN):						 */
struct gpio_register_field {
	uint8_t b0:1;
	uint8_t b1:1;
	uint8_t b2:1;
	uint8_t b3:1;
	uint8_t b4:1;
	uint8_t b5:1;
	uint8_t b6:1;
	uint8_t b7:1;
};
/* rfr2 Registers */

/********************* USAGE ***************************/
/*	status = TRX_status.TRX_Status;                    */
/*	while (!TRX_status.CCA_Done)                       */
/*		;                                              */
/*******************************************************/

// Transceiver Pin Register
struct TRX_pr {
	uint8_t reset:1;
	uint8_t sleep:1;
	uint8_t test:1;
	uint8_t enableAT:1;
	uint8_t reserved:4;
};
#define TRX_pr (*((volatile struct TRX_pr*)&TRXPR))

struct TRX_status {
	uint8_t TRX_Status:5;
	uint8_t TST_Status:1;
	uint8_t CCA_Status:1;
	uint8_t CCA_Done:1;
};
#define TRX_status (*((volatile struct TRX_status*)&TRX_STATUS))

struct TRX_state {
	uint8_t TRX_Cmd:5;
	uint8_t TRAC_Status:3;
};
#define TRX_state (*((volatile struct TRX_state*)&TRX_STATE))

struct TRX_ctrl0 {
	uint8_t ctrl0:8;
};
#define TRX_ctrl0 (*((volatile struct TRX_ctrl0*)&TRX_CTRL_0))

struct TRX_ctrl1 {
		uint8_t Reserved:4;
		uint8_t PLL_TX_Flt:1;
		uint8_t TX_AUTO_CRC_On:1;
		uint8_t IRQ_2_EXT_En:1;
		uint8_t PA_EXT_En:1;
};
#define TRX_ctrl1 (*((volatile struct TRX_ctrl1*)&TRX_CTRL_1))

struct PHY_TX_pwr {
	uint8_t Pwr:4;
	uint8_t res:4;
};
#define PHY_TX_pwr (*((volatile struct PHY_TX_pwr*)&PHY_TX_PWR))

// Receiver Signal Strength indicator
struct PHY_rssi	{
	uint8_t Rssi:5;
	uint8_t RND_Value:2;
	uint8_t CRC_Valid:1;
};
#define PHY_rssi (*((volatile struct PHY_rssi*)&PHY_RSSI))

struct PHY_ED_level {
	uint8_t ED_level:8;
};			// Energy Detection Level
#define PHY_ED_level (*((volatile struct PHY_ED_level*)&PHY_ED_LEVEL))

struct PHY_CC_cca {
	uint8_t Channel:5;
	uint8_t CCA_Mode:2;
	uint8_t CCA_Request:1;
};			// Transceiver Clear Channel Assessment (CCA) control register
#define PHY_CC_cca (*((volatile struct PHY_CC_cca*)&PHY_CC_CCA))

// Transceiver CCA Threshold Setting Register
struct CCA_thres {
	uint8_t ED_threshold:4;
	uint8_t CS_threshold:4;
};
#define CCA_thres (*((volatile struct CCA_thres*)&CCA_THRES))

// Transceiver Receive Control register
struct RX_ctrl {
	uint8_t PDT_Thres:4;
	uint8_t Soft_Mode:1;
	uint8_t ACR_Mode:1;
	uint8_t SDM_Mode:2;
};
#define RX_ctrl (*((volatile struct RX_ctrl*)&RX_CTRL))

// Start of Frame Delimiter register
struct SFD_value {
	uint8_t SFD_value:8;
};
#define SFD_value (*((volatile struct SFD_value*)&SFD_VALUE))

// Transceiver Control Register 2
struct	TRX_ctrl2 {
	uint8_t OQPSK_DATA_Rate:2;
	uint8_t Reserved:5;
	uint8_t RX_SAFE_Mode:1;
};			
#define TRX_ctrl2 (*((volatile struct TRX_ctrl2*)&TRX_CTRL_2))

// Antenna Diversity control register
struct ANT_div {
	uint8_t ANT_Ctrl:2;
	uint8_t ANT_EXT_SW_En:1;
	uint8_t ANT_DIV_En:1;
	uint8_t Reserved:3;
	uint8_t ANT_Sel:1;
};
#define ANT_div (*((volatile struct ANT_div*)&ANT_DIV))

// Transceiver Interrupt Enable Register
struct	IRQ_mask {
	uint8_t PPL_LOCK_Enable:1;
	uint8_t PPL_UNLOCK_Enable:1;
	uint8_t RX_START_Enable:1;
	uint8_t RX_END_Enable:1;
	uint8_t CCA_ED_DONE_Enable:1;
	uint8_t AMI_Enable:1;
	uint8_t TX_END_Enable:1;
	uint8_t AWAKE_Enable:1;
};
#define IRQ_mask (*((volatile struct IRQ_mask*)&IRQ_MASK))

// Transceiver Interrupt Status Register
struct	IRQ_status {
	uint8_t PLL_Lock:1;
	uint8_t PLL_Unlock:1;
	uint8_t RX_Start:1;
	uint8_t RX_End:1;
	uint8_t CCA_ED_Done:1;
	uint8_t AMI_Status:1;
	uint8_t TX_End:1;
	uint8_t Awake:1;
};
#define IRQ_status (*((volatile struct IRQ_status*)&IRQ_STATUS))

// Transceiver Interrupt Status Register 1
struct	IRQ_status1 {
	uint8_t TX_Start:1;
	uint8_t AMI_Maf0:1;	// Address Match Interrupt (Match Address Filter 0)
	uint8_t AMI_Maf1:1;	// Address Match Interrupt (Match Address Filter 1)
	uint8_t AMI_Maf2:1;	// Address Match Interrupt (Match Address Filter 2)
	uint8_t AMI_Maf3:1;	// Address Match Interrupt (Match Address Filter 3)
	uint8_t Reserved:3;
};
#define IRQ_status1 (*((volatile struct IRQ_status1*)&IRQ_STATUS1))

struct	VREG_ctrl {
	uint8_t DVREG_Trim:2;
	uint8_t DVDD_Ok:1;
	uint8_t DVREG_Ext:1;
	uint8_t AVREG_Trim:2;
	uint8_t AVDD_Ok:1;
	uint8_t AVREG_Ext:1;
};			// Voltage Regulator Control and Status Register
#define VREG_ctrl (*((volatile struct VREG_ctrl*)&VREG_CTRL))

// Battery Monitor Control and Status Register
struct PHY_batmon {
	uint8_t BATMON_Vth:4;
	uint8_t BATMON_Hr:1;
	uint8_t BATMON_Ok:1;
	uint8_t BAT_LOW_Enable:1;
	uint8_t BAT_Low:1;
};
#define PHY_batmon (*((volatile struct PHY_batmon*)&BATMON))

// Crystal Oscillator Control Register
struct XOSC_ctrl {
	uint8_t XTAL_Trim:4;
	uint8_t XTAL_Mode:4;
};
#define XOSC_ctrl (*((volatile struct XOSC_ctrl*)&XOSC_CTRL))

// Transceiver Receiver Sensitivity Control Register
struct RX_syn {
	uint8_t RX_PDT_Level:4;
	uint8_t RXO_Cfg:2;
	uint8_t RX_Overide:1;
	uint8_t RX_PDT_Disable:1;
};
#define RX_syn (*((volatile struct RX_syn*)&RX_SYN))

struct XAH_ctrl1 {
		uint8_t Reserved0:1;
		uint8_t AACK_PROM_Mode:1;
		uint8_t AACK_ACK_Time:1;
		uint8_t Reserved3:1;
		uint8_t AACK_UPLD_RES_Ft:1;
		uint8_t AACK_FLTR_RES_Ft:1;
		uint8_t Reserved6:2;
};			// Tranceiver Acknowledgement Frame Control Register
#define XAH_ctrl1 (*((volatile struct XAH_ctrl1*)&XAH_CTRL_1))

// Filter TuNing Control
struct FTN_ctrl {
	uint8_t ftnv:6;
	uint8_t ftn_ROUND:1;
	uint8_t ftn_start:1;
};
#define FTN_ctrl (*((volatile struct FTN_ctrl*)&FTN_CTRL))

// Center Frequency calibration control register
struct PLL_cf{
	uint8_t PLL_cf:4;
	uint8_t VMOD_tune:2;
	uint8_t enable:1;
	uint8_t start:1;
};
#define PLL_cf (*((volatile struct PLL_cf*)&PLL_CF))

// Delay Cell Unit
struct PLL_dcu {
	uint8_t dcuw:6;
	uint8_t Reserved:1;
	uint8_t start:1;
};
#define PLL_dcu (*((volatile struct PLL_dcu*)&PLL_DCU))

// Device Identification Register (Part Number)
struct PART_num {
	uint8_t PART_num:8;
};
#define PART_num (*((volatile struct PART_num*)&PART_NUM))

// Device Identification Register (Version Number)
struct VERSION_num {
	uint8_t  num:8;
};
#define VERSION_num (*((volatile struct VERSION_num*)&VERSION_NUM))

// Device Identification Register (Manufacture ID Low Byte)
struct MAN_id0 {
	uint8_t id:8;
};
#define MAN_id0 (*((volatile struct MAN_id0*)&MAN_ID_0))

// Device Identification Register (Manufacture ID High Byte)
struct MAN_id1 {
	uint8_t id:8;
};
#define MAN_id1 (*((volatile struct MAN_id1*)&MAN_ID_1))

// Transceiver MAC Short Address Register (Low Byte)
struct SHORT_addr0 {
	uint8_t address:8;
};
#define SHORT_addr0 (*((volatile struct SHORT_addr0*)&SHORT_ADDR_0))

// Transceiver MAC Short Address Register (High Byte)
struct SHORT_addr1 {
	uint8_t address:8;
};
#define SHORT_addr1 (*((volatile struct SHORT_addr1*)&SHORT_ADDR_1))

// Transceiver Personal Area Network ID Register (Low Byte)
struct PAN_id0 {
	uint8_t id:8;
};
#define PAN_id0 (*((volatile struct PAN_id0*)&PAN_ID_0))

// Transceiver Personal Area Network ID Register (High Byte)
struct PAN_id1 {
	uint8_t id:8;
};
#define PAN_id1 (*((volatile struct PAN_id1*)&PAN_ID_1))

// Transceiver MAC IEEE Address Register 0
struct IEEE_addr0 {
	uint8_t address:8;
};
#define IEEE_addr0 (*((volatile struct IEEE_addr0*)&IEEE_ADDR_0))

// Transceiver MAC IEEE Address Register 1
struct IEEE_addr1 {
	uint8_t address:8;
};
#define IEEE_addr1 (*((volatile struct IEEE_addr1*)&IEEE_ADDR_1))
	
// Transceiver MAC IEEE Address Register 2
struct IEEE_addr2 {
	uint8_t address:8;
};
#define IEEE_addr2 (*((volatile struct IEEE_addr2*)&IEEE_ADDR_2))

// Transceiver MAC IEEE Address Register 3
struct IEEE_addr3 {
	uint8_t address:8;
};
#define IEEE_addr3 (*((volatile struct IEEE_addr3*)&IEEE_ADDR_3))
	
// Transceiver MAC IEEE Address Register 4
struct IEEE_addr4 {
	uint8_t address:8;
};
#define IEEE_addr4 (*((volatile struct IEEE_addr4*)&IEEE_ADDR_4))
	
// Transceiver MAC IEEE Address Register 5
struct IEEE_addr5 {
	uint8_t address:8;
};
#define IEEE_addr5 (*((volatile struct IEEE_addr5*)&IEEE_ADDR_5))
	
// Transceiver MAC IEEE Address Register 6
struct IEEE_addr6 {
	uint8_t address:8;
};
#define IEEE_addr6 (*((volatile struct IEEE_addr6*)&IEEE_ADDR_6))
	
// Transceiver MAC IEEE Address Register 7
struct IEEE_addr7 {
	uint8_t address:8;
};
#define IEEE_addr7 (*((volatile struct IEEE_addr7*)&IEEE_ADDR_7))

// Transceiver Extended Operating Mode Control Register
struct XAH_ctrl0 {
	uint8_t SLOTTED_Operation:1;
	uint8_t MAX_CSMA_Retries:3;
	uint8_t MAX_FRAME_Retries:4;
};
#define XAH_ctrl0 (*((volatile struct XAH_ctrl0*)&XAH_CTRL_0))

// Transceiver CSMA-CA Random Number Generator Seed Register
struct CSMA_seed0 {
	uint8_t value:8;
};
#define CSMA_seed0 (*((volatile struct CSMA_seed0*)&CSMA_SEED_0))

// Transceiver Acknowledgment Frame Control Register 2
struct CSMA_seed1 {
	uint8_t value:3;
	uint8_t AACK_I_AM_Coord:1;
	uint8_t AACK_DIS_Ack:1;
	uint8_t AACK_SET_Pd:1;
	uint8_t AACK_FVN_Mode:2;
};
#define CSMA_seed1 (*((volatile struct CSMA_seed1*)&CSMA_SEED_1))

// Transceiver CSMA-CA Back-off Exponent Control Register
struct CSMA_be {
	uint8_t MIN_Be:4;
	uint8_t MAX_Be:4;
};
#define CSMA_be (*((volatile struct CSMA_be*)&CSMA_BE))

// Power Amplifier Ramp up/down Control register
struct	PARCr {
	uint8_t PARUfi:1;
	uint8_t PARDfi:1;
	uint8_t PALTu:3;
	uint8_t PALTd:3;
};
#define PARCr (*((volatile struct PARCr*)&PARCR))

// Transceiver Interrupt Enable Register 1
struct	IRQ_Mask1 {
	uint8_t TX_START_Enable:1;
	uint8_t MAF_0_AMI_Enable:1;
	uint8_t MAF_1_AMI_Enable:1;
	uint8_t MAF_2_AMI_Enable:1;
	uint8_t MAF_3_AMI_Enable:1;
	uint8_t Reserved0:1;
	uint8_t Reserved1:1;
	uint8_t Reserved2:1;
};
#define IRQ_Mask1 (*((volatile struct IRQ_Mask1*)&IRQ_MASK1))

#endif
