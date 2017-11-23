#include "precompiledHeader.h"
#include "MyOpenGLWindow.h"
#include "MyMesh.h"
#include "MyRayTracer.h"

void checkGLError() {
	auto e = glGetError();
	if (e != GL_NO_ERROR) {
		printf("glGetError %d\n", e);
	}	
}

class Scoped_glEnable {
public:
	Scoped_glEnable(GLenum v) {
		m_v = v;
		glEnable(m_v);
	}

	~Scoped_glEnable() {
		glDisable(m_v);
	}

private:
	GLenum m_v;
};

class Scoped_glColor {
public:
	Scoped_glColor(float r, float g, float b, float a) {
		glColor4f(r,g,b,a);
	}
	~Scoped_glColor() {
		glColor4f(1,1,1,1);
	}
};

class Scoped_glPushMatrix {
public:
	Scoped_glPushMatrix() {
		glPushMatrix();
	}
	~Scoped_glPushMatrix() {
		glPopMatrix();
	}
};

class MyDemoWindow : public MyOpenGLWindow {
public:
	virtual void onGLinit() override {
		m_mesh.loadObjFile("models/test.obj");
		createDisplayNormals();
	}

	void createDisplayNormals() {
		float normalLength = 0.2f;

		auto n = m_mesh.vertices.size();
		m_displayNormals.resize(n * 2);
		auto* dst = m_displayNormals.data();
		int i = 0;
		for (auto& v : m_mesh.vertices) {
			*dst = v.pos; dst++;
			*dst = v.pos + v.normal * normalLength; dst++;
		}
	}

	virtual void onDestroy() override {
		PostQuitMessage(0);
	}

	virtual void onMouseEvent(MouseEvent& ev) override {
		switch (ev.type) {
			case MouseEventType::MouseMove: {
				if (ev.rightButton) {
					float dx = ev.x - m_mouseLastPosX;
					float dy = ev.y - m_mouseLastPosY;

					float s = 0.25f;

					m_cameraY += dx * s;
					m_cameraX += dy * s;
				}
				if (ev.middleButton) {
					float dx = ev.x - m_mouseLastPosX;
					m_fovy += dx * 0.025f;
				}
			}break;
			case MouseEventType::MouseWheel: {
				m_cameraDistance += ev.zDelta * 0.01f;
				if (m_cameraDistance < 0.01f)
					m_cameraDistance = 0.01f;
			}break;

			case MouseEventType::LButtonDown: {
				rayTracing(ev.x, ev.y);
			}break;
		}

		m_mouseLastPosX = ev.x;
		m_mouseLastPosY = ev.y;
	}

	void rayTracing(float x, float y) {
		MyMatrix4f projMatrix;
		MyMatrix4f modelview;

		glGetFloatv(GL_PROJECTION_MATRIX, projMatrix.data);
		glGetFloatv(GL_MODELVIEW_MATRIX,  modelview.data);

		MyVec2f screenSize(static_cast<float>(canvasWidth()), static_cast<float>(canvasHeight()));
		m_rayTracer.init(screenSize, projMatrix, modelview);
		auto ray = m_rayTracer.getRay(x, y);

		m_debugRay = ray;

		MyTriangle tri(	MyVec3f( 0,0,1),
						MyVec3f( 1,0,0),
						MyVec3f(-1,0,0));

		MyRay3f::HitResult result;
		if (ray.raycast(result, tri, result.distance)) {
			m_debugPoint = result.point;
			std::cout << "debug Point " << m_debugPoint << "\n";
		}
	}

	void drawGrid() {
		glColor4f(0.5f, 0.5f, 0.5f, 1);
		glLineWidth(1);
		glBegin(GL_LINES);
			for (float x = -10; x <= 10; x++) {
				glVertex3f( x, 0,-10);
				glVertex3f( x, 0, 10);
			}

			for (float z = -10; z <= 10; z++) {
				glVertex3f(-10, 0, z);
				glVertex3f( 10, 0, z);
			}
		glEnd();
		glColor4f(1,1,1,1);
	}

	void drawOriginAxis() {
		glLineWidth(2);
		glBegin(GL_LINES);
			glColor4f(1,0,0,1); glVertex3f(0,0,0); glVertex3f(1,0,0);
			glColor4f(0,1,0,1); glVertex3f(0,0,0); glVertex3f(0,1,0);
			glColor4f(0,0,1,1); glVertex3f(0,0,0); glVertex3f(0,0,1);
		glEnd();
		glColor4f(1,1,1,1);
		glLineWidth(1);
	}

	void initCamera() {
		float aspect = static_cast<float>(canvasWidth()) / canvasHeight();
		glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(m_fovy, aspect, 0.01, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_DEPTH_TEST);
		
		glTranslatef(0, 0, -m_cameraDistance);
		glRotatef(m_cameraX, 1,0,0);
		glRotatef(m_cameraY, 0,1,0);
	}

	void drawDisplayNormals() {
		if (!m_displayNormals.size())
			return;

		Scoped_glColor color(1,1,0,1);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, m_displayNormals[0].data);

		glDrawArrays(GL_LINES, 0, m_displayNormals.size());

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void example1(float uptime) {
		m_mesh.wireframe = true;
		m_mesh.draw();
		//drawDisplayNormals();

		{
			glPointSize(10);
			glColor4f(1,0,0,1);
			glBegin(GL_POINTS);
				glVertex3fv(m_debugPoint.data);
			glEnd();
		}
		{
			glColor4f(1,0,1,1);
			glBegin(GL_POINTS);
				glVertex3fv(m_debugRay.origin.data);
			glEnd();
			glBegin(GL_LINES);
				glVertex3fv(m_debugRay.origin.data);
				auto v = m_debugRay.origin + m_debugRay.direction;
				glVertex3fv(v.data);
			glEnd();
		}
	}

	virtual void onPaint() override {
		auto uptime = static_cast<float>(m_uptime.get());

		glViewport(0, 0, canvasWidth(), canvasHeight());

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glColor4f(1,1,1,1);

		glClearColor(0.0f, 0.2f, 0.2f, 0);
		glClearDepth(1);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLoadIdentity();

		//---------------
		initCamera();
		drawGrid();
		drawOriginAxis();

		example1(uptime);

		swapBuffers();
	}

	MyHiResTimer m_uptime;
	float m_cameraX = 30;
	float m_cameraY = 30;
	float m_cameraDistance = 4.0f;

	float m_fovy = 60;

	float m_mouseLastPosX;
	float m_mouseLastPosY;

	MyMesh m_mesh;

	std::vector<MyVec3f> m_displayNormals;

	MyRayTracer m_rayTracer;
	MyVec3f m_debugPoint;
	MyRay3f m_debugRay;
};

int main() {
	MyDemoWindow w;
	w.create();

    MSG msg;

#if 0 // using PeekMessage
	for(;;) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);  
			DispatchMessage(&msg);  
		}else{
			w.onPaint();
		}
	}
#else
	while(GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);  
		DispatchMessage(&msg);  
	}	
#endif

	return msg.wParam;
}