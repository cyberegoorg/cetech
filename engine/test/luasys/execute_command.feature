Feature: Luasys console server command 'lua_system.execute'

  Scenario Outline: Can call lua expresion and recive return value in response
    Given New instance
    When call lua cmd <cmd>
    Then Response is <response>

    Examples:
      | cmd                            | response |
      | return 1+1                     | 2        |
      | return 'aaa'                   | aaa      |
      | return {a=1}                   | {'a': 1} |
      | return function() return 1 end | function |
      | return nil                     | None     |
