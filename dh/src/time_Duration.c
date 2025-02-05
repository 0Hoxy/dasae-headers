#include "dh/time/Duration.h"
#include "dh/core/prim/flt.h"
#include "dh/debug/assert.h"
#include "dh/time/common.h"


time_Duration time_Duration_from(u64 secs, u32 nanos) {
    return literal_time_Duration_from(secs, nanos);
}

time_Duration time_Duration_fromSecs(u64 secs) {
    return literal_time_Duration_fromSecs(secs);
}

time_Duration time_Duration_fromMillis(u64 millis) {
    return literal_time_Duration_fromMillis(millis);
}

time_Duration time_Duration_fromMicros(u64 micros) {
    return literal_time_Duration_fromMicros(micros);
}

time_Duration time_Duration_fromNanos(u64 nanos) {
    return literal_time_Duration_fromNanos(nanos);
}

u64 time_Duration_asSecs(time_Duration self) {
    return self.secs;
}

u32 time_Duration_subsecMillis(time_Duration self) {
    return self.nanos / time_nanos_per_milli;
}

u32 time_Duration_subsecMicros(time_Duration self) {
    return self.nanos / time_nanos_per_micro;
}

u32 time_Duration_subsecNanos(time_Duration self) {
    return self.nanos;
}

time_Duration time_Duration_fromSecs_f64(f64 secs) {
    return time_Duration_from(
        as$(u64, secs),
        as$(u32, (secs - as$(f64, as$(u64, secs))) * as$(f64, time_nanos_per_sec))
    );
}

f64 time_Duration_asSecs_f64(time_Duration self) {
    return as$(f64, self.secs) + as$(f64, self.nanos) / as$(f64, time_nanos_per_sec);
}

time_Duration op_fnAdd(time_Duration) {
    return unwrap(time_Duration_addChecked(self, other));
}

time_Duration op_fnSub(time_Duration) {
    return unwrap(time_Duration_subChecked(self, other));
}

time_Duration op_fnMulBy(time_Duration, u64) {
    return unwrap(time_Duration_mulChecked_u64(self, other));
}

time_Duration op_fnDivBy(time_Duration, u64) {
    return unwrap(time_Duration_divChecked_u64(self, other));
}

bool time_Duration_isZero(time_Duration self) {
    return time_Duration_eq(self, time_Duration_zero);
}

Opt$time_Duration time_Duration_addChecked(time_Duration lhs, time_Duration rhs) {
    reserveReturn(Opt$time_Duration);
    let total_nanos = (lhs.secs * time_nanos_per_sec + lhs.nanos)
                    + (rhs.secs * time_nanos_per_sec + rhs.nanos);
    // Check for overflow in nanoseconds
    if (total_nanos < lhs.nanos || total_nanos < rhs.nanos
        || (0 < lhs.secs && (u64_limit - lhs.secs) < rhs.secs)) {
        return_none();
    }
    let secs  = total_nanos / time_nanos_per_sec;
    let nanos = as$(u32, total_nanos % time_nanos_per_sec);
    return_some(literal_time_Duration_from(secs, nanos));
}

Opt$time_Duration time_Duration_subChecked(time_Duration lhs, time_Duration rhs) {
    reserveReturn(Opt$time_Duration);
    let lhs_total_nanos = lhs.secs * time_nanos_per_sec + lhs.nanos;
    let rhs_total_nanos = rhs.secs * time_nanos_per_sec + rhs.nanos;
    // Check for underflow
    if (lhs_total_nanos < rhs_total_nanos) {
        return_none();
    }
    let diff_nanos = lhs_total_nanos - rhs_total_nanos;
    let secs       = diff_nanos / time_nanos_per_sec;
    let nanos      = as$(u32, diff_nanos % time_nanos_per_sec);
    return_some(literal_time_Duration_from(secs, nanos));
}

Opt$time_Duration time_Duration_mulChecked_u64(time_Duration lhs, u64 rhs) {
    reserveReturn(Opt$time_Duration);
    // Check for overflow
    if ((u32_limit - rhs) < lhs.nanos || (u64_limit / rhs) < lhs.secs) {
        return_none();
    }
    let total_nanos = lhs.secs * rhs + lhs.nanos;
    return_some(literal_time_Duration_from(total_nanos / time_nanos_per_sec, total_nanos % time_nanos_per_sec));
}

Opt$time_Duration time_Duration_divChecked_u64(time_Duration lhs, u64 rhs) {
    reserveReturn(Opt$time_Duration);
    // Check for division by zero or overflow
    if (rhs == 0 || (u64_limit / rhs) < lhs.secs || (u32_limit / rhs) < lhs.nanos) {
        return_none();
    }
    let total_nanos = lhs.secs * rhs + lhs.nanos;
    let secs        = total_nanos / time_nanos_per_sec;
    let nanos       = as$(u32, total_nanos % time_nanos_per_sec);
    return_some(literal_time_Duration_from(secs, nanos));
}
