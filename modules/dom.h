// the document object model
// for browsers
struct HtmlElement {
  char* innerHTML;
  struct HtmlElement* child; //YAY - linked list
  struct HtmlElement* parent;
};

struct Document {
  char* URL;
  struct HtmlElement* child;
};

struct Document newdoc(){
  struct Document o;
  o.URL = (char*)"about:newtab";
  return o;
}

void freedoc(struct Document o){
  o.URL = "";
  o.child = 0; 
}

void addchild(struct HtmlElement parent,struct HtmlElement* u){
  parent.child = u;
  u->parent = &parent;
}
