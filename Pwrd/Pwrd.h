#pragma once

#include "resource.h"
#include <vector>
#include <string>
#include <ctime>
// Structure for password entry
struct PasswordEntry {
    std::wstring name;
    std::wstring website;
    std::wstring email;
    std::wstring user;
    std::wstring password;
     std::wstring note;
    
     COLORREF color  ;
    std::wstring category; // Added for category support
    std::wstring creationDate;
};

 


 