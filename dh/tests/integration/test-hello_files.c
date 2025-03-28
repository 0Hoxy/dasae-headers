#include "dh/core.h"
#include "dh/Arr.h"
#include "dh/Str.h"
#include <stdio.h>

use_Ptr$(FILE);
use_Sli$(Ptr$FILE);
use_Arr$(4, Ptr$FILE);

config_ErrSet(LoadFileErr,
    FailedOpenFile
);

use_Err$(Ptr$FILE);
pvt fn_(loadFile(Str_const filename),must_check Err$Ptr$FILE);
use_Err$(Arr_const$4$Ptr$FILE);
pvt fn_(loadFiles(void), $must_check Err$Arr_const$4$Ptr$FILE);

fn_ext_scope(loadFile(Str_const filename), Err$Ptr$FILE) {
    if_(let file = fopen(as$(const char*, filename.ptr), "r"),
        file != null) {
        return_ok(file);
    }
    return_err(LoadFileErr_FailedOpenFile());
} ext_unscoped;

fn_ext_scope(loadFiles(void), Err$Arr_const$4$Ptr$FILE) {
    FILE* fp1 = fopen("hello1.txt", "r");
    if (fp1 == null) {
        return_err(LoadFileErr_FailedOpenFile());
    }
    errdefer_($ignore fclose(fp1));

    let fp2 = try_(loadFile(Str_l("hello2.txt")));
    errdefer_($ignore fclose(fp2));

    let fp3 = try_(loadFile(Str_l("hello3.txt")));
    errdefer_($ignore fclose(fp3));

    let fp4 = try_(loadFile(Str_l("hello4.txt")));
    errdefer_($ignore fclose(fp4));

    return_ok({ fp1, fp2, fp3, fp4 });
} ext_unscoped;
