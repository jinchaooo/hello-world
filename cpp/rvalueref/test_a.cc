#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <utility>

class hello 
{
  public:
    int* a_;
    hello() 
    {
      std::cout << "construct object: default constructor\n";
      a_ = new int(1);
    }
    hello(const hello& hh) 
    {
      std::cout << "construct object: copy constructor\n";
      a_ = new int(*hh.a_);
    }
    hello(hello&& hh) 
    {
      std::cout << "construct object: move constructor\n";
      a_ = hh.a_;
      hh.a_ = NULL;
    }
    ~hello()
    {
      std::cout << "destruct object\n";
      if (a_)
        delete a_;
      else
        std::cout << "destruct: NULL pointer\n";
    }
};

// pass by value
void set_hello(hello h) 
{
  std::cout << "enter set_hello\n";
  *h.a_ = 5;
}

// pass by reference
void set_hello_2(hello& h) 
{
  std::cout << "enter set_hello_2\n";
  *h.a_ = 10;
}

// pass by rvalue reference
void set_hello_3(hello&& h) 
{
  std::cout << "enter set_hello_3\n";
  *h.a_ = 15;
}

int main()
{
  std::cout << "enter main\n";
  std::cout << "main(): construct h with no parameter (default)\n";
  hello h;

  /**
   * non-reference, copy constructor
   */
  std::cout << "main(): call set_hello(hello)\n";
  set_hello(h);
  std::cout << "main(): *h.a_ = " << *h.a_ << std::endl;

  /**
   * lvalue reference
   */
  std::cout << "main(): call set_hello_2(hello&)\n";
  set_hello_2(h);
  std::cout << "main(): *h.a_ = " << *h.a_ << std::endl;

  /**
   * compile error: rvalue reference cannot take lvalue
   */
  //set_hello_3(h);
  //std::cout << "*h.a_ = " << *h.a_ << std::endl;

  /**
   * rvalue reference will not change life time of object
   */
  std::cout << "main(): call set_hello_3(hello&&)\n";
  set_hello_3(std::move(h));
  std::cout << "main(): *h.a_ = " << *h.a_ << std::endl;

  std::cout << "main(): construct hh by std::move(h)\n";
  hello hh(std::move(h));
  std::cout << "main(): *hh.a_ = " << *hh.a_ << std::endl;

  /**
   * runtime error: NULL pointer
   */
  //std::cout << "main(): *h.a_ = " << *h.a_ << std::endl;

  /**
   * compiler will automatically call copy constructor
   */
  std::cout << "main(): construct hhh by =hh\n";
  hello hhh = hh;
  std::cout << "main(): *hhh.a_ = " << *hhh.a_ << std::endl;

  /**
   * compiler will automatically call move constructor
   */
  std::cout << "main(): construct hhhh by =std::move(hh)\n";
  hello hhhh = std::move(hh);
  std::cout << "main(): *hhhh.a_ = " << *hhhh.a_ << std::endl;

  /**
   * compiler will call move constructor
   */
  std::cout << "main(): construct hhhhh by std::make_shared\n";
  std::shared_ptr<hello> phhhhh = std::make_shared<hello>(std::move(hhh));
  std::cout << "main(): phhhhh->a_ = " << *(phhhhh->a_) << std::endl;
}
