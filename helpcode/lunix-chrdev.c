/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * Vasilis (indorilftw) Gerakaris <vgerak@gmail.com>
 * Gregory (mastergreg) Liras <gregliras@gmail.com>
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;
	
	WARN_ON ( !(sensor = state->sensor));
	/* TODO */
    if ( sensor->msr_data[state->type]->last_update > state->buf_timestamp ) {
        return 1;
    }

	/* The following return is bogus, just for the stub to compile */
	return 0; /* TODO */
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor;
    unsigned long sflags;
    unsigned long newdata;
    uint32_t data;
    uint32_t timestamp;
    const char device_str[ N_LUNIX_MSR ][ LUNIX_CHRDEV_BUFSZ ] = { "Battery", "Temp", "Light" };

	
	debug("leaving\n");

	WARN_ON ( !(sensor = state->sensor));

	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	/* TODO */
	/* Why use spinlocks? See LDD3, p. 119 */
    spin_lock_irqsave(&sensor->lock, sflags);
    /* reader dragon here */
    newdata=lunix_chrdev_state_needs_refresh(state);
    if ( newdata == 1 ) {
        data = sensor->msr_data[state->type]->values[0];
        timestamp = sensor->msr_data[state->type]->last_update;
    }
    spin_unlock_irqrestore(&sensor->lock, sflags);
    /* ok i got the data */


	/*
	 * Any new data available?
	 */
	/* TODO */



	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */

	/* TODO */
    if ( newdata==1 ) {
        down(&state->lock);

        snprintf( state -> buf_data, LUNIX_CHRDEV_BUFSZ, "%s: %u\n", device_str[state->type], data );
        state -> buf_data[ LUNIX_CHRDEV_BUFSZ-1 ]='\0';
        state -> buf_timestamp = timestamp;
        state -> buf_lim = strnlen( state -> buf_data, LUNIX_CHRDEV_BUFSZ );

        up( & state -> lock );
        debug( "chill, i got this: %d\n", data ) ;
    }

	debug("leaving\n");
    return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	/* Declarations */
	/* TODO */
	int ret;
    dev_t minor;
    int type;
    struct lunix_chrdev_state_struct *state;

	debug("entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */

    minor = iminor( inode );
    type = minor & 7;
	
	/* Allocate a new Lunix character device private state structure */
	/* TODO */
    state = kmalloc( sizeof( struct lunix_chrdev_state_struct) , GFP_USER );
    if ( !state ) {
        debug( "open:failed to allocate resource\n" ) ;
        goto out;
    }

    state -> type = type < N_LUNIX_MSR ? type : 0;
    state -> buf_lim = 0;
    state -> sensor = &lunix_sensors[( minor >> 3 )];
    state -> buf_timestamp = 0;

    sema_init( &state->lock, 1 );

    debug( "open: allocation complete \n" ) ;
    
    filp->private_data = state;

out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	/* TODO */
    /*
     * FIXME:
     * do we need to release anything here?
     * possible memory leak exists 
     * on multiple calls to open
     */
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Why? */
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{
	ssize_t ret;

	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	state = filp->private_data;
	if (WARN_ON(!state) ) {
        ret = -EINVAL;
        goto out;
    }

	sensor = state->sensor;
	if (WARN_ON(!sensor) ) {
        ret = -EINVAL;
        goto out;
    }

	/* Lock? */
    debug( "trying to read\n" ) ;

    /*
     *  we cannot lock here, 
     *  if we do update cannot work properly
     */

	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
        DEFINE_WAIT( mwait );
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* TODO 
			   The process needs to sleep 
			   See LDD3, page 153 for a hint */
            prepare_to_wait( &sensor -> wq, &mwait, TASK_INTERRUPTIBLE );
            schedule();
            finish_wait( &sensor -> wq, &mwait );
            /*
             * it should be interruptible
             * the spinlock spins
             */
		}
	}

    debug( "read some data\n" ) ;

    /*
     * now we can lock
     */
    down( &state->lock ) ;

	/* End of file */
	/* TODO */
    if ( *f_pos >= state -> buf_lim ) {
        ret = 0;
        goto out;
    }
	
	/* Determine the number of cached bytes to copy to userspace */
	/* TODO */

    cnt = cnt > state -> buf_lim ? state -> buf_lim : cnt;
    if ( ( ret = copy_to_user( usrbuf, state -> buf_data, cnt ) ) < 0 ) {
        ret = -EFAULT;
        goto out;
    }

    debug( "state -> buf_data = %s, usrbuf = %s" , state -> buf_data, usrbuf );

    ret = cnt - ret;
    *f_pos += ret;

	/* Auto-rewind on EOF mode? */
	/* TODO */
out:
	/* Unlock? */
    up( &state->lock ) ;
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = 
{
    .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

int lunix_chrdev_init(void)
{
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
	
	debug("initializing character device\n");
    /* initialize the chardev */
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);

    /* FIXME: isn't this set in the initialization?
     * line 178 ? */
	lunix_chrdev_cdev.owner = THIS_MODULE;
	
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	/* TODO */
	/* register_chrdev_region? */
    ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix" );
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}	
	/* TODO */
	/* cdev_add? */
    ret = cdev_add( &lunix_chrdev_cdev, dev_no, lunix_minor_cnt );
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
		
	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}
