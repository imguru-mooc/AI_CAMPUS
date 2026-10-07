/* mysysfs.c — /sys/kernel/mysysfs/ 에 속성 두 개를 만드는 커널 모듈
 *   value  (읽기·쓰기, 0664) : 정수 하나
 *   writes (읽기 전용, 0444) : value 에 쓴 횟수
 * 빌드: cd module && make          (linux-headers-$(uname -r) 필요)
 * 적재: sudo insmod mysysfs.ko && sudo dmesg | tail -2
 * 제거: sudo rmmod mysysfs
 */
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>

static struct kobject *my_kobj;
static int value;
static int writes;

/* cat value 하면 불림 — buf 에 텍스트를 쓰고 길이를 돌려준다 */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%d\n", value);
}

/* echo 42 > value 하면 불림 — buf 의 텍스트를 해석한다 */
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    int ret = kstrtoint(buf, 10, &value);
    if (ret < 0)
        return ret;                 /* 숫자가 아니면 -EINVAL → 사용자에게 오류 */
    writes++;
    pr_info("mysysfs: value = %d (쓰기 %d 번째)\n", value, writes);
    return count;                   /* 다 받았다고 알림 */
}

static ssize_t writes_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%d\n", writes);
}

static struct kobj_attribute value_attr  = __ATTR(value, 0664, value_show, value_store);
static struct kobj_attribute writes_attr = __ATTR(writes, 0444, writes_show, NULL);

static struct attribute *my_attrs[] = {
    &value_attr.attr,
    &writes_attr.attr,
    NULL,                           /* 목록의 끝 */
};

static const struct attribute_group my_group = {
    .attrs = my_attrs,
};

static int __init mysysfs_init(void)
{
    int ret;
    my_kobj = kobject_create_and_add("mysysfs", kernel_kobj);   /* /sys/kernel/mysysfs */
    if (!my_kobj)
        return -ENOMEM;
    ret = sysfs_create_group(my_kobj, &my_group);
    if (ret) {
        kobject_put(my_kobj);
        return ret;
    }
    pr_info("mysysfs: /sys/kernel/mysysfs 생성\n");
    return 0;
}

static void __exit mysysfs_exit(void)
{
    kobject_put(my_kobj);           /* 디렉터리와 속성이 함께 사라짐 */
    pr_info("mysysfs: 제거\n");
}

module_init(mysysfs_init);
module_exit(mysysfs_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("AI_CAMPUS Linux & Shell");
MODULE_DESCRIPTION("sysfs attribute example");
