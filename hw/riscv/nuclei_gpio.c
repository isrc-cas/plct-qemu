#include "qemu/osdep.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/riscv/nuclei_gpio.h"
#include "migration/vmstate.h"
#include "trace.h"

static uint64_t nuclei_gpio_read(void *opaque, hwaddr offset, unsigned int size)
{
   NucLeiGPIOState *s =NUCLEI_GPIO(opaque);
    uint64_t value = 0;

    switch (offset) {
    case  NUCLEI_GPIOx_CTL0:
        value = s->gpiox_ctl0;
        break;
    case  NUCLEI_GPIOx_CTL1:
        value = s->gpiox_ctl1;
        break;
    case  NUCLEI_GPIOx_ISTAT:
        value = s->gpiox_istat;
        break;
    case  NUCLEI_GPIOx_OCTL:
        value = s->gpiox_octl;
        break;
    case  NUCLEI_GPIOx_BOP:
        value = s->gpiox_bop;
        break;
    case  NUCLEI_GPIOx_BC:
        value = s->gpiox_bc;
        break;
    case  NUCLEI_GPIOx_LOCK:
        value = s->gpiox_lock;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                "%s: bad read offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
    }
    return value;
}

static void nuclei_gpio_write(void *opaque, hwaddr offset,
                       uint64_t value, unsigned int size)
{
   NucLeiGPIOState *s =NUCLEI_GPIO(opaque);

    switch (offset) {
    case  NUCLEI_GPIOx_CTL0:
        s->gpiox_ctl0 = value;
        break;
    case  NUCLEI_GPIOx_CTL1:
        s->gpiox_ctl1 = value;
        break;
    case  NUCLEI_GPIOx_ISTAT:
         s->gpiox_istat = value;
        break;
    case  NUCLEI_GPIOx_OCTL:
        s->gpiox_octl = value;
        break;
    case  NUCLEI_GPIOx_BOP:
        s->gpiox_bop = value;
        break;
    case  NUCLEI_GPIOx_BC:
        s->gpiox_bc = value;
        break;
    case  NUCLEI_GPIOx_LOCK:
        s->gpiox_lock = value;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: bad write offset 0x%" HWADDR_PRIx "\n",
                      __func__, offset);
    }
}

static const MemoryRegionOps gpio_ops = {
    .read =  nuclei_gpio_read,
    .write = nuclei_gpio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .impl.min_access_size = 4,
    .impl.max_access_size = 4,
};

static void nuclei_gpio_reset(DeviceState *dev)
{
   NucLeiGPIOState *s =NUCLEI_GPIO(dev);

    s->gpiox_ctl0 = 0x44444444;
    s->gpiox_ctl1 = 0x44444444;
    s->gpiox_istat = 0x0;
    s->gpiox_octl = 0x0;
    s->gpiox_bop = 0x0;
    s->gpiox_bc = 0x0;
    s->gpiox_lock = 0x0;
}

static void nuclei_gpio_init(Object *obj)
{
   NucLeiGPIOState *s =NUCLEI_GPIO(obj);

    memory_region_init_io(&s->mmio, obj, &gpio_ops, s,
            TYPE_NUCLEI_GPIO,NUCLEI_GPIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio);
}

static void nuclei_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = nuclei_gpio_reset;
    dc->desc = "Nuclei GPIO";
}

static const TypeInfo nuclei_gpio_info = {
    .name = TYPE_NUCLEI_GPIO,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(NucLeiGPIOState),
    .instance_init = nuclei_gpio_init,
    .class_init = nuclei_gpio_class_init
};

static void nuclei_gpio_register_types(void)
{
    type_register_static(&nuclei_gpio_info);
}

type_init(nuclei_gpio_register_types)
