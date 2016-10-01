from pytest_bdd import scenario, given, when, then


@scenario(
    'execute_command.feature',
    'Can call lua expresion and recive return value in response',
    example_converters=dict(cmd=str))
def test_execute_command():
    pass


@given("New instance")
def new_instance(engine_instance):
    return dict(engine=engine_instance)


@when('call lua cmd <cmd>')
def execute_command(new_instance, cmd):
    new_instance["cmd_ret_value"] = new_instance["engine"].lua_execute(cmd)['response']


@then('Response is <response>')
def response_is(new_instance, response):
    cmd_ret_value = new_instance["cmd_ret_value"]

    if cmd_ret_value is None:
        assert str(cmd_ret_value) == response
    elif type(cmd_ret_value) is dict:
        assert str(cmd_ret_value) == response
    else:
        assert cmd_ret_value == type(cmd_ret_value)(response)
