///*
// * Template file, not actually used in the project.
// * Copy file when creating a new application and add app functionality
// */
//#include "app_template.h"
//#include "gui.h"
//
//static const uint16_t imagedata[1024] = {
//		/* Add Imagedata here */
//};
//
//static Image_t icon = { .width = 32, .height = 32, .data = imagedata };
//
///* Forward declaration of actual app task */
//static void Template(void *unused);
//
///* Creates the app task, called from desktop when user start the app */
//static void Template_Start(){
//	xTaskCreate(Template, "Supply", 300, NULL, 3, NULL);
//}
//
///* Register this app */
//void Template_Init() {
//	App_Register("Template", Template_Start, icon);
//}
//
//static void Template(void *unused) {
//	/* Create GUI elements */
//	container_t *c= container_new(COORDS(280, 240));
//	c->base.position.x = 40;
//
//	/* Notify desktop of started app */
//	desktop_AppStarted(Template_Start, (widget_t*) c);
//
//	while(1) {
//		uint32_t signal;
//		if (App_Handler(&signal, 300)) {
//			/* Handle app signals */
//		}
//	}
//}
