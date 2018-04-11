/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   icon_resume_png;
    const int            icon_resume_pngSize = 340;

    extern const char*   icon_stop_png;
    const int            icon_stop_pngSize = 336;

    extern const char*   icon_pause_png;
    const int            icon_pause_pngSize = 340;

    extern const char*   icon_play_png;
    const int            icon_play_pngSize = 1014;

    extern const char*   icon_record_png;
    const int            icon_record_pngSize = 926;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 5;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}
