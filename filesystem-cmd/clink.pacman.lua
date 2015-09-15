-- CMD shell integration for MSYS2
--
-- Copyright (C) 2015 Stefan Zimmermann <zimmermann.code@gmail.com>
--
-- Licensed under the Apache License, Version 2.0


new_parser = clink.arg.new_parser

-- dummy parser for disabling completion
local none = new_parser({})

-- create an override parser doing default filesystem matching
function new_default_parser()
    return new_parser({function() end})
end

local default = new_default_parser({function() end})


-- auto-completion of package names
local packages_parser = new_parser()
-- the actual packages table to be used in pacman argument parser
local packages = (function()
    local p = io.popen("pacman.exe -Ssq 2>&1")
    local names = {}
    for name in p:lines() do
        -- let package args recursively expect another package as next arg
        table.insert(names, name .. packages_parser)
    end
    if p:close() then
       return names
    end
    return {}
end)()
packages_parser:set_arguments(packages)


-- auto-completion for different types of values expected by subflags
value_parsers = {}
value_parsers["dir"] = default
value_parsers["path"] = default
value_parsers["repo"] = new_parser({"msys", "mingw32", "mingw64"})
value_parsers["when"] = new_parser({"auto", "always", "never"})

-- get available subflags from pacman <flag> --help
local function subflags(flag)
    local p = io.popen("pacman.exe " .. flag .. " --help 2>&1")
    local subflags = {}
    for line in p:lines() do
        --   -x, --long <value type>   description...
        local short, long, value_type = line:match(
          "^%s*(%-[^%s])%s*,%s*(%-%-[^%s]+)%s+<([^>]+)>")
        if not value_type then
            --   -x, --long   description...
            short, long = line:match("^%s*(%-[^%s])%s*,%s*(%-%-[^%s]+)")
            if not short then
               --   --long <value type>   description...
               long, value_type = line:match("^%s*(%-%-[^%s]+)%s+<([^>]+)>")
               if not value_type then
                   --   --long   description...
                   long = line:match("^%s*(%-%-[^%s]+)")
               end
            end
        end
        local value_parser = nil
        if value_type then
            value_parser = value_parsers[value_type] or none
        end
        if short then
            table.insert(subflags,
              value_parser and short .. value_parser or short)
        end
        if long then
            table.insert(subflags,
              value_parser and long .. value_parser or long)
        end
    end
    if p:close() then
        return subflags
    end
    return {}
end


clink.arg.register_parser("pacman",
  -- also set packages as base arguments
  -- to enable package completion after combined short flags
  -- which are not explicitly defined (like -Syu)
  new_parser(packages):set_flags({
      -- flags without further args
      "-h" .. none, "--help" .. none,
      "-V" .. none, "--version" .. none,
  }):add_flags((function()
      local flags = {}
      -- flags followed by optional subflags and package names
      for _, flag in next, {
          "-D", "--database",
          "-F", "--files",
          "-Q", "--query",
          "-R", "--remove",
          "-S", "--sync",
          "-T", "--deptest",
      } do
          table.insert(flags, flag ..
            new_parser(packages):set_flags(subflags(flag)))
      end
      -- flags followed by optional subflags and file paths
      local subflags = subflags("-U")
      for _, flag in next, {"-U", "--upgrade"} do
          table.insert(flags, flag ..
            new_default_parser():set_flags(subflags))
      end
      return flags
  end)())
)
