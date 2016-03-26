// ReSharper disable UnusedMember.Local
using NSpec;

namespace CETech.Tests
{
    public class entitymanager_spec: nspec
    {
        void given_entity_manager()
        {
            before = () => EntityManager.Init();
            after = () => EntityManager.Shutdown();

            context["When create new entity"] = () =>
            {
                int ent1 = 0;

                before = () => ent1 = EntityManager.Create();
                it["New entity is alive"] = () => EntityManager.Alive(ent1).should_be_true();

                context["When destroy entity"] = () =>
                {
                    before = () => EntityManager.Destroy(ent1);
                    it["Entity is not alive"] = () => EntityManager.Alive(ent1).should_be_false();
                };
            };
        }
    }
}