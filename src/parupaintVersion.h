#define PARUPAINT_MAJOR_VERSION 0
#define PARUPAINT_MINOR_VERSION 9
#define PARUPAINT_PATCH_VERSION 2

#define PARUPAINT_VERSION__(ma,mi,pa) #ma "." #mi #pa
#define PARUPAINT_VERSION_(ma,mi,pa) PARUPAINT_VERSION__(ma, mi, pa)
#define PARUPAINT_VERSION PARUPAINT_VERSION_(PARUPAINT_MAJOR_VERSION, PARUPAINT_MINOR_VERSION, PARUPAINT_PATCH_VERSION)
