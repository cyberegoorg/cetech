from pytest_bdd import scenario, given, when, then


@scenario(
    'execute_command.feature',
    'Can call lua expresion and recive return value in response',
    example_converters=dict(cmd=str))
def test_execute_command():
    pass


@given("New instance")
def new_instance(engine_instance):
    return engine_instance


@given('Lua cmd <cmd> return value')
def execute_command(new_instance, cmd):
    return new_instance.lua_execute(cmd)['response']


@then('Response is <response>')
def response_is(execute_command, response):
    if execute_command is None:
        assert str(execute_command) == response
    elif type(execute_command) is dict:
        assert str(execute_command) == response
    else:
        assert execute_command == type(execute_command)(response)
