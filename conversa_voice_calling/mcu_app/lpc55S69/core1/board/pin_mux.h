/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_13_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 5. */
#define PIO0_13_FUNC_ALT5 0x05u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_19_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 7. */
#define PIO0_19_FUNC_ALT7 0x07u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_20_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 7. */
#define PIO0_20_FUNC_ALT7 0x07u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_21_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 7. */
#define PIO0_21_FUNC_ALT7 0x07u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_26_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO0_26_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_29_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO0_29_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO0_30_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO0_30_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_10_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_10_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_11_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_11_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_13_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 2. */
#define PIO1_13_FUNC_ALT2 0x02u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_20_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 5. */
#define PIO1_20_FUNC_ALT5 0x05u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_21_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 5. */
#define PIO1_21_FUNC_ALT5 0x05u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_23_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO1_23_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_25_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO1_25_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_31_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 1. */
#define PIO1_31_FUNC_ALT1 0x01u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_4_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_4_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_5_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_5_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_6_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_6_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_7_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_7_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_8_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_8_FUNC_ALT0 0x00u
/*!
 * @brief Select Digital mode.: Enable Digital mode. Digital input is enabled. */
#define PIO1_9_DIGIMODE_DIGITAL 0x01u
/*!
 * @brief Selects pin function.: Alternative connection 0. */
#define PIO1_9_FUNC_ALT0 0x00u

/*! @name PIO1_4 (number 1), LED_
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_LED_B_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_B_GPIO_PIN_MASK (1U << 4U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_LED_B_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_B_PIN 4U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_LED_B_PIN_MASK (1U << 4U)      /*!<@brief PORT pin mask */
                                                            /* @} */

/*! @name PIO1_6 (number 5), LED_R
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_LED_R_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_R_GPIO_PIN_MASK (1U << 6U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_LED_R_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_R_PIN 6U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_LED_R_PIN_MASK (1U << 6U)      /*!<@brief PORT pin mask */
                                                            /* @} */

/*! @name PIO1_7 (number 9), LED_G
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_LED_G_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_G_GPIO_PIN_MASK (1U << 7U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_LED_G_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_LED_G_PIN 7U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_LED_G_PIN_MASK (1U << 7U)      /*!<@brief PORT pin mask */
                                                            /* @} */

/*! @name PIO1_5 (number 31), DbgPin0
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_DbgPin0_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin0_GPIO_PIN_MASK (1U << 5U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_DbgPin0_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin0_PIN 5U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_DbgPin0_PIN_MASK (1U << 5U)      /*!<@brief PORT pin mask */
                                                              /* @} */

/*! @name PIO1_8 (number 24), DbgPin1
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_DbgPin1_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin1_GPIO_PIN_MASK (1U << 8U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_DbgPin1_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin1_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_DbgPin1_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                              /* @} */

/*! @name PIO1_9 (number 10), BtnS3
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_BtnS3_GPIO GPIO                /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_BtnS3_GPIO_PIN_MASK (1U << 9U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_BtnS3_PORT 1U                  /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_BtnS3_PIN 9U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_BtnS3_PIN_MASK (1U << 9U)      /*!<@brief PORT pin mask */
                                                            /* @} */

/*! @name PIO1_10 (number 40), DbgPin3
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_DbgPin3_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin3_GPIO_PIN_MASK (1U << 10U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_DbgPin3_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin3_PIN 10U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_DbgPin3_PIN_MASK (1U << 10U)      /*!<@brief PORT pin mask */
                                                               /* @} */

/*! @name PIO1_11 (number 93), DbgPin2
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_INITPINS_CORE1_DbgPin2_GPIO GPIO                 /*!<@brief GPIO peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin2_GPIO_PIN_MASK (1U << 11U) /*!<@brief GPIO pin mask */
#define BOARD_INITPINS_CORE1_DbgPin2_PORT 1U                   /*!<@brief PORT peripheral base pointer */
#define BOARD_INITPINS_CORE1_DbgPin2_PIN 11U                   /*!<@brief PORT pin number */
#define BOARD_INITPINS_CORE1_DbgPin2_PIN_MASK (1U << 11U)      /*!<@brief PORT pin mask */
                                                               /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins_Core1(void); /* Function assigned for the Cortex-M33 (Core #1) */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/