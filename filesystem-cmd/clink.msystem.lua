-- CMD shell integration for MSYS2
--
-- Copyright (C) 2015 Stefan Zimmermann <zimmermann.code@gmail.com>
--
-- Licensed under the Apache License, Version 2.0


new_parser = clink.arg.new_parser

-- dummy parser for disabling completion
local none = new_parser({})

-- dummy parser falling back to clink's default filesystem matching
--TODO: somehow predefined in clink api?
local default = new_parser({function() end})


local systems = {
    "msys" .. none,
    "mingw32" .. none,
    "mingw64" .. none,
}

-- specifiers for msystem /i
local installers = {
    -- supports an optional clink settings dir
    "clink" .. default,
}

clink.arg.register_parser("msystem",
  new_parser({systems}):set_flags({
      "/?" .. none,
      "/d" .. none,
      "/i" .. new_parser(installers),
  })
)
