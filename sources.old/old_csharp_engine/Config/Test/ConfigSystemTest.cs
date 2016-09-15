// ReSharper disable UnusedMember.Local

using NSpec;

namespace CETech.Tests
{
    public class ConfigSystem_spec: nspec
    {
        void given_new_config_system()
        {
            before = () => ConfigSystem.Init();
            after = () => ConfigSystem.Shutdown();

            context["when create int value"] = () =>
            {
                before = () =>{ ConfigSystem.CreateValue("name", "description", 1); };

                it["then value is 1"] = () => ConfigSystem.Int("name").should_be(1);
                it["then type is int"] = () => ConfigSystem.GetValueType("name").should_be(ConfigSystem.ConfigValueType.Int);
                
                context["when change value"] = () =>
                {
                    before = () => { ConfigSystem.SetValue("name", 2); };

                    it["then value is changed"] = () => ConfigSystem.Int("name").should_be(2);
                };
            };

            context["when create float value"] = () =>
            {
                before = () => { ConfigSystem.CreateValue("name", "description", 2.0f); };

                it["then value is 2.0"] = () => ConfigSystem.Float("name").should_be(2.0f);
                it["then type is float"] = () => ConfigSystem.GetValueType("name").should_be(ConfigSystem.ConfigValueType.Float);

                context["when change value"] = () =>
                {
                    before = () => { ConfigSystem.SetValue("name", 1.0f); };

                    it["then value is changed"] = () => ConfigSystem.Float("name").should_be(1.0f);
                };
            };

            context["when create string value"] = () =>
            {
                before = () => { ConfigSystem.CreateValue("name", "description", "value"); };

                it["then value is \"value\""] = () => ConfigSystem.String("name").should_be("value");
                it["then type is string"] = () => ConfigSystem.GetValueType("name").should_be(ConfigSystem.ConfigValueType.String);

                context["when change value"] = () =>
                {
                    before = () => { ConfigSystem.SetValue("name", "new_value"); };

                    it["then value is changed"] = () => ConfigSystem.String("name").should_be("new_value");
                };
            };

        }
    }
}