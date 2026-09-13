// RUN: %clang_cc1 -triple ia16-unknown-none-elf -ffreestanding -emit-llvm -o - %s | FileCheck %s

struct one_byte { unsigned char a; };
struct three_bytes { unsigned char a[3]; };
struct five_bytes { unsigned char a[5]; };

signed char return_char(signed char value) { return value; }
int return_int(int value) { return value; }
long return_long(long value) { return value; }
long long return_long_long(long long value) { return value; }
float return_float(float value) { return value; }
double return_double(double value) { return value; }
struct one_byte return_one(struct one_byte value) { return value; }
struct three_bytes return_three(struct three_bytes value) { return value; }
struct five_bytes return_five(struct five_bytes value) { return value; }

// CHECK: define{{.*}} signext i8 @return_char(i8{{.*}} signext %value)
// CHECK: define{{.*}} i16 @return_int(i16{{.*}} %value)
// CHECK: define{{.*}} i32 @return_long(i32{{.*}} %value)
// CHECK: define{{.*}} void @return_long_long(ptr{{.*}} sret(i64){{.*}}, i64{{.*}} %value)
// CHECK: define{{.*}} float @return_float(float{{.*}} %value)
// CHECK: define{{.*}} void @return_double(ptr{{.*}} sret(double){{.*}}, double{{.*}} %value)
// CHECK: define{{.*}} i16 @return_one(i16 %value.coerce)
// CHECK: define{{.*}} i32 @return_three(i32 %value.coerce)
// CHECK: define{{.*}} void @return_five(ptr{{.*}} sret(%struct.five_bytes){{.*}}, ptr{{.*}} byval(%struct.five_bytes){{.*}} %value)
