using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace tritTest
{
    class Program
    {
        static void Main(string[] args)
        {
            bool bContinue = true;
            while (bContinue)
            {
                Console.WriteLine("Enter trit: ");
                string s = Console.ReadLine();
                if (s.Equals("q"))
                {
                    bContinue = false;
                }
                else
                {
                    double value = 0;
                    bool wasError = false;
                    double powOf = 0;
                    for (int i = s.Length - 1; i >= 0; --i, powOf++)
                    {
                        switch (s[i])
                        {
                            case '+':
                                value += Math.Pow(3, powOf);
                                break;
                            case '-':
                                value -= Math.Pow(3, powOf);
                                break;
                            case '0':
                                //do nothing
                                break;
                            default:
                                i = 0;
                                Console.WriteLine("invalidate character:");
                                wasError = true;
                                break;
                        }
                    }
                    if (!wasError)
                    {
                        Console.WriteLine("Decimal value is: {0:0}", value);
                    }
                }
            }
        }
    }
}
