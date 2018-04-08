#include <string>
#include <sstream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>

using namespace Urho3D;
/**
* Using the convenient Application AM_PI we don't have
* to worry about initializing the engine or writing a main.
* You can probably mess around with initializing the engine
* and running a main manually, but this is convenient and portable.
*/
class MyApp : public Application
{
public:
    int framecount_;
    float time_;
    SharedPtr<Text> text_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> boxNode_;
	static const int res = 200;
	Node* lines[res];
	StaticModel* objects[res];
	Node* grid[res*res];
	StaticModel* surface[res*res];
	SharedPtr<Node> cameraNode_;
	ResourceCache* cache;


    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    MyApp(Context * context) : Application(context),framecount_(0),time_(0)
    {
    }

    /**
    * This method is called before the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup()
    {
        // These parameters should be self-explanatory.
        // See http://urho3d.github.io/documentation/1.5/_main_loop.html
        // for a more complete list.
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
    }

	Vector3 Quadratic(float x)
	{
		const float scaleY = 2.0f / res;
		return Vector3(x, x*x*scaleY, res);
	}

	Vector3 Linear(float x)
	{
		const float scaleY = 2.0f / res;
		return Vector3(x, x*scaleY, res);
	}

	Vector3 Sin(float amplitude, float phase, float freq, float x) {
		float real_x = x * 10 * M_PI / res;
		float y = amplitude * sin(freq * real_x + phase);
		return Vector3(real_x, y, res);
	}

	Vector3 Sine(float x, float z, float t)
	{
		Vector3 p;
		float y = sin(M_PI * (x + t));
		return Vector3(x, y, z);
	}

	Vector3 MultiSine(float x, float z, float t)
	{
		float y = sin(M_PI * (x + t));
		y += sin(2.0f * M_PI * (x + 2.0f * t)) / 2.0f;
		y *= 2.0f / 3.0f;
		Vector3 p;
		return Vector3(x, y, z);
	}

	Vector3 Sine2D(float x, float  z, float t)
	{
		float sinx = sin(M_PI * (x + t));
		float sinz = sin(M_PI * (z + t));
		Vector3 p;
		float y = (sinx + sinz) * 0.5f;
		return Vector3(x, y, z);
	}

	Vector3 MultiSine2DFunction(float x, float z, float t)
	{
		float y = 4.0f * sin(M_PI * (x + z + t * 0.5f));
		y += sin(M_PI * (x + t));
		y += sin(2.0f * M_PI * (z + 2.0f * t)) * 0.5f;
		y *= 1.0f / 5.5f;
		Vector3 p;
		return Vector3(x, y, z);
	}

	Vector3 Ripple(float x, float z, float t)
	{
		float d = sqrt(x * x + z * z);
		float y = sin(M_PI * (4.0f * d - t));
		y /= 1.0f + 10.0f + d;
		return Vector3(x, y, z);
	}

	Vector3 Cylinder(float u, float v, float t)
	{
		float radius = 0.8f + sin(M_PI * (6.0f * u + 2.0f * v + t)) * 0.2f;
		Vector3 p;
		float x = radius * sin(M_PI * u);
		float y = v;
		float z = radius * cos(M_PI * u);
		return Vector3(x, y, z);
	}

	Vector3 Sphere(float u, float v, float t)
	{
		Vector3 p;
		float r = 0.8f + sin(M_PI * (6.0f * u + t)) * 0.1f;
		r += sin(M_PI * (4.0f * v + t)) * 0.1f;
		float s = r * cos(M_PI * 0.5f * v);
		float x = s * sin(M_PI * u);
		float y = r * sin(M_PI * 0.5f * v);
		float z = s * cos(M_PI * u);
		return Vector3(x, y, z);
	}

	Vector3 Torus(float u, float v, float t)
	{
		Vector3 p;
		float r1 = 0.65f + sin(M_PI * (6.0f * u + t)) * 0.1f;
		float r2 = 0.2f + sin(M_PI * (4.0f * v + t)) * 0.05f;
		float s = r2 * cos(M_PI * v) + r1;
		float x = s * sin(M_PI * u);
		float y = r2 * sin(M_PI * v);
		float z = s * cos(M_PI * u);
		return Vector3(x, y, z);
	}



    /**
    * This method is called after the engine has been initialized.
    * This is where you set up your actual content, such as scenes,
    * models, controls and what not. Basically, anything that needs
    * the engine initialized and ready goes in here.
    */
	virtual void Start()
	{
		// We will be needing to load resources.
		// All the resources used in this example comes with Urho3D.
		// If the engine can't find them, check the ResourcePrefixPath (see http://urho3d.github.io/documentation/1.5/_main_loop.html).
		cache = GetSubsystem<ResourceCache>();

		// Let's use the default style that comes with Urho3D.
		GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

		scene_ = new Scene(context_);
		// Let the scene have an Octree component!
		scene_->CreateComponent<Octree>();
		// Let's add an additional scene component for fun.
		scene_->CreateComponent<DebugRenderer>();

		//Create the sky background
		//Find out how to change the background
		Node* skyNode = scene_->CreateChild("Sky");
		skyNode->SetScale(500.0f); //The scale does not matter
		Skybox* skybox = skyNode->CreateComponent<Skybox>();
		skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
		/*
		for (int i = 0; i < res; i++)
		{
			lines[i] = scene_->CreateChild("Box");
			float val = i - (res / 2);
			lines[i]->SetPosition(Sin(5,0,1,val));
			int scale = 1;
			lines[i]->SetScale(Vector3(scale, scale, scale));
			objects[i] = lines[i]->CreateComponent<StaticModel>();
			objects[i]->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
			objects[i]->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
			objects[i]->SetCastShadows(false);
		}
		*/

		for (int i = 0; i < res*res; i++)
		{
			float step = 2.0f / res;
			grid[i] = scene_->CreateChild("Box");
			grid[i]->SetPosition(Vector3(0, 0, 0));
			grid[i]->SetScale(Vector3(step, step, step));
			surface[i] = grid[i]->CreateComponent<StaticModel>();
			surface[i]->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
			surface[i]->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
			surface[i]->SetCastShadows(false);
		}

		// We need a camera from which the viewport can render.
		cameraNode_ = scene_->CreateChild("Camera");
		Camera* camera = cameraNode_->CreateComponent<Camera>();
		camera->SetFarClip(2000);

		//#Find out which lights are needed
		// Create a red directional light (sun)
		{
			Node* lightNode = scene_->CreateChild();
			lightNode->SetDirection(Vector3::FORWARD);
			lightNode->Yaw(50);     // horizontal
			lightNode->Pitch(10);   // vertical
			Light* light = lightNode->CreateComponent<Light>();
			light->SetLightType(LIGHT_DIRECTIONAL);
			light->SetBrightness(1.6);
			light->SetColor(Color(1.0, .6, 0.3, 1));
			light->SetCastShadows(true);
		}
		// Create a blue point light
		{
			Node* lightNode = scene_->CreateChild("Light");
			lightNode->SetPosition(Vector3(-10, 2, 5));
			Light* light = lightNode->CreateComponent<Light>();
			light->SetLightType(LIGHT_POINT);
			light->SetRange(25);
			light->SetBrightness(1.7);
			light->SetColor(Color(0.5, .5, 1.0, 1));
			light->SetCastShadows(true);
		}
		// add a green spot light to the camera node
		{
			Node* node_light = cameraNode_->CreateChild();
			Light* light = node_light->CreateComponent<Light>();
			node_light->Pitch(15);  // point slightly downwards
			light->SetLightType(LIGHT_SPOT);
			light->SetRange(20);
			light->SetColor(Color(.6, 1, .6, 1.0));
			light->SetBrightness(2.8);
			light->SetFov(25);
		}

		// Now we setup the viewport. Of course, you can have more than one!
		Renderer* renderer = GetSubsystem<Renderer>();
		SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
		renderer->SetViewport(0, viewport);

		//#Find out which 
		// We subscribe to the events we'd like to handle.
		// In this example we will be showing what most of them do,
		// but in reality you would only subscribe to the events
		// you really need to handle.
		// These are sort of subscribed in the order in which the engine
		// would send the events. Read each handler method's comment for
		// details.
		SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(MyApp, HandleBeginFrame));
		SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));
		SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(MyApp, HandlePostUpdate));
		SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(MyApp, HandleRenderUpdate));
		SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MyApp, HandlePostRenderUpdate));
		SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(MyApp, HandleEndFrame));
    }

    /**
    * Good place to get rid of any system resources that requires the
    * engine still initialized. You could do the rest in the destructor,
    * but there's no need, this method will get called when the engine stops,
    * for whatever reason (short of a segfault).
    */
    virtual void Stop()
    {
    }

    /**
    * Every frame's life must begin somewhere. Here it is.
    */
    void HandleBeginFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }

    /**
    * Input from keyboard is handled here. I'm assuming that Input, if
    * available, will be handled before E_UPDATE.
    */
    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
		using namespace KeyDown;
		int key = eventData[P_KEY].GetInt();
		if (key == KEY_ESCAPE)
			engine_->Exit();

		if (key == KEY_TAB)    // toggle mouse cursor when pressing tab
		{
			GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
			GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
		}
    }

    /**
    * You can get these events from when ever the user interacts with the UI.
    */
    void HandleClosePressed(StringHash eventType,VariantMap& eventData)
    {
        engine_->Exit();
    }
    /**
    * Your non-rendering logic should be handled here.
    * This could be moving objects, checking collisions and reaction, etc.
    */
    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
        framecount_++;
        time_+=timeStep;
        // Movement speed as world units per second
        float MOVE_SPEED=10.0f;
        // Mouse sensitivity as degrees per M_PIxel
        const float MOUSE_SENSITIVITY=0.1f;

        if(time_ >=1)
        {
            std::string str;
            str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
            {
                std::ostringstream ss;
                ss<<framecount_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" frames in ");
            {
                std::ostringstream ss;
                ss<<time_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" seconds = ");
            {
                std::ostringstream ss;
                ss<<(float)framecount_/time_;
                std::string s(ss.str());
                str.append(s.substr(0,6));
            }
            str.append(" fps");
            String s(str.c_str(),str.size());
            //text_->SetText(s);
            URHO3D_LOGINFO(s);     // this show how to put stuff into the log
            framecount_=0;
            //time_=0;
        }

        // Rotate the box thingy.
        // A much nicer way of doing this would be with a LogicComponent.
        // With LogicComponents it is easy to control things like movement
        // and animation from some IDE, console or just in game.
        // Alas, it is out of the scope for our simple example.
        //boxNode_->Rotate(Quaternion(8*timeStep,16*timeStep,0));

        Input* input=GetSubsystem<Input>();
        if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
            MOVE_SPEED*=10;
        if(input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

        if(!GetSubsystem<Input>()->IsMouseVisible())
        {
            // Use this frame's mouse motion to adjust camera node yaw and M_PItch. Clamp the M_PItch between -90 and 90 degrees
            IntVector2 mouseMove=input->GetMouseMove();
            static float yaw_=0;
            static float pitch_=0;
            yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
            pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
            pitch_=Clamp(pitch_,-90.0f,90.0f);
            // Reset rotation and set yaw and M_PItch again
            cameraNode_->SetDirection(Vector3::FORWARD);
            cameraNode_->Yaw(yaw_);
            cameraNode_->Pitch(pitch_);
        }
		/*
		for (int i = 0; i < res; i++)
		{
			//lines[i] = scene_->CreateChild("Box");
			float val = i - (res / 2);
			//Sin(5, time_, 1, val)
			lines[i]->SetPosition(Quadratic(val));
			int scale = 1;
			lines[i]->SetScale(Vector3(scale, scale, scale));
		}*/
		float step = 2.0f / res;
		float t = time_;
		for (int z = 0, i = 0; z < res; z++)
		{
			float v = (z + 0.5f) * step - 1.0f;
			for (int x = 0; x < res; x++, i++)
			{
				float u = (x + 0.5f) * step - 1.0f;
				grid[i]->SetPosition(Torus(u, v, t));
			}
		}
    }
    /**
    * Anything in the non-rendering logic that requires a second pass,
    * it might be well suited to be handled here.
    */
    void HandlePostUpdate(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * If you have any details you want to change before the viewport is
    * rendered, try putting it here.
    * See http://urho3d.github.io/documentation/1.32/_rendering.html
    * for details on how the rendering M_PIpeline is setup.
    */
    void HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * After everything is rendered, there might still be things you wish
    * to add to the rendering. At this point you cannot modify the scene,
    * only post rendering is allowed. Good for adding things like debug
    * artifacts on screen or brush up lighting, etc.
    */
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We could draw some debuggy looking thing for the octree.
        // scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
    }
    /**
    * All good things must come to an end.
    */
    void HandleEndFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
};

/**
* This macro is expanded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)