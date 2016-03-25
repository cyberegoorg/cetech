using System;
using NSpec;

namespace CETech.Tests
{
    public class EntityManagerTest: nspec
    {


        void before_each() { Console.WriteLine("before"); name = "NSpec"; }

        void after_each() { Console.WriteLine("after"); }

        void it_works()
        {
            name.should_be("NSpec");
        }

        void describe_nesting()
        {
            before = () => name += " BDD";

            it["works here"] = () => name.should_be("NSpec BDD");

            it["and here"] = () => name.should_be("NSpec BDD");
        }
        string name;
    }
}