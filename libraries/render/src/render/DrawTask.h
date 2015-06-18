//
//  DrawTask.h
//  render/src/render
//
//  Created by Sam Gateau on 5/21/15.
//  Copyright 20154 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_Task_h
#define hifi_render_Task_h

#include "Engine.h"

namespace render {

template <class T> void jobRun(T& jobModel, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) {
    jobModel.run(sceneContext, renderContext);
}
template <class T, class I> void jobRunI(T& jobModel, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const I& input) {
    jobModel.run(sceneContext, renderContext, input);
}
template <class T, class O> void jobRunO(T& jobModel, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, O& output) {
    jobModel.run(sceneContext, renderContext, output);
}
template <class T, class I, class O> void jobRunIO(T& jobModel, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const I& input, O& output) {
    jobModel.run(sceneContext, renderContext, input, output);
}

class Job {
public:

    // Varying represent a varying piece of data
    class Varying {
    public:

        Varying(const Varying& var): _concept(var._concept) {}

        Varying() {}
        template <class T> 
        Varying(const T& data) : _concept(new Job::Varying::Model<T>(data)) {}

        // Access the _data contained win the concept explicitely
        template <class T> T& edit() { return (static_cast<Model<T>*> (_concept.get())->_data); }
        template <class T> const T& get() const { return (static_cast<const Model<T>*> (_concept.get())->_data); }

    protected:
        friend class Job;

        std::vector<std::weak_ptr<Job>> _consumerJobs;

        void addJobConsumer(const std::shared_ptr<Job>& job) {
            _consumerJobs.push_back(job);
        }
 
        class Concept {
        public:
            virtual ~Concept() = default;
        };
        template <class T> class Model : public Concept {
        public:
            typedef T Data;
            Data _data;
            Model(const Model& source): _data(source.data) {}
            Model(const Data& data): _data(data) {}
            virtual ~Model() {}
        };

        std::shared_ptr<Concept> _concept;
    };

    Job(const Job& other) : _concept(other._concept) {}
    ~Job();

    const std::string& getName() const { return _concept->getName(); }
    const Varying getInput() const { return _concept->getInput(); }
    const Varying getOutput() const { return _concept->getOutput(); }

    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) {
        if (_concept) {
            _concept->run(sceneContext, renderContext);
        }
    }

protected:
public:

    class Concept {
        std::string _name;
    public:
        Concept() : _name() {}
        Concept(const std::string& name) : _name(name) {}
        virtual ~Concept() = default;
        
        void setName(const std::string& name) { _name = name; }
        const std::string& getName() const { return _name; }
        
        virtual const Varying getInput() const { return Varying(); }
        virtual const Varying getOutput() const { return Varying(); }
        virtual void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) = 0;
    };

    Job(Concept* concept) : _concept(concept) {}

public:
    template <class T> class Model : public Concept {
    public:
        typedef T Data;

        Data _data;

        Model() {}
        Model(const std::string& name): Concept(name) {}
        Model(Data data): _data(data) {}
        Model(Data data, const std::string& name): Concept(name), _data(data) {}

        void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) { jobRun(_data, sceneContext, renderContext); }
    };

    template <class T, class I> class ModelI : public Concept {
    public:
        typedef T Data;
        typedef I Input;

        Data _data;
        Varying _input;

        const Varying getInput() const { return _input; }

        ModelI(const Varying& input): _input(input) {}
        ModelI(const Varying& input, const std::string& name): Concept(name), _input(input) {}
        ModelI(Data data): _data(data) {}

        void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) { jobRunI(_data, sceneContext, renderContext, _input.get<I>()); }
    };

    template <class T, class O> class ModelO : public Concept {
    public:
        typedef T Data;
        typedef O Output;

        Data _data;
        Varying _output;

        const Varying getOutput() const { return _output; }

        ModelO() : _output(Output()) {
            
        }

        ModelO(Data data): _data(data), _output(Output()) {}

        void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) {
            jobRunO(_data, sceneContext, renderContext, _output.edit<O>());
        }
    };

    template <class T, class I, class O> class ModelIO : public Concept {
    public:
        typedef T Data;
        typedef I Input;
        typedef O Output;

        Data _data;
        Varying _input;
        Varying _output;

        const Varying getInput() const { return _input; }
        const Varying getOutput() const { return _output; }

        ModelIO(const Varying& input): _input(input), _output(Output()) {}
        ModelIO(Data data, Output output): _data(data), _output(output) {}

        void setInput(const Varying& input) { _input = input; }

        void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext) { jobRunIO(_data, sceneContext, renderContext, _input.get<I>(), _output.edit<O>()); }
    };

    std::shared_ptr<Concept> _concept;
};




typedef std::vector<Job> Jobs;

void cullItems(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const ItemIDsBounds& inItems, ItemIDsBounds& outITems);
void depthSortItems(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, bool frontToBack, const ItemIDsBounds& inItems, ItemIDsBounds& outITems);
void renderItems(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const ItemIDsBounds& inItems, int maxDrawnItems = -1);


class FetchItems {
public:

    FetchItems() {}
    FetchItems(const ItemFilter& filter): _filter(filter) {}

    ItemFilter _filter = ItemFilter::Builder::opaqueShape().withoutLayered();

    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, ItemIDsBounds& outItems);

    typedef Job::ModelO<FetchItems, ItemIDsBounds> JobModel;
};

class CullItems {
public:
    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const ItemIDsBounds& inItems, ItemIDsBounds& outItems);

    typedef Job::ModelIO<CullItems, ItemIDsBounds, ItemIDsBounds> JobModel;
};

class DepthSortItems {
public:
    bool _frontToBack = true;

    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const ItemIDsBounds& inItems, ItemIDsBounds& outITems);

    typedef Job::ModelIO<DepthSortItems, ItemIDsBounds, ItemIDsBounds> JobModel;
};



class DrawOpaque {
public:
};
template <> void jobRun(DrawOpaque& job, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);


class DrawTransparent {
public:
};
template <> void jobRun(DrawTransparent& job, const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

class DrawLight {
public:
    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

    typedef Job::Model<DrawLight> JobModel;
};

class DrawBackground {
public:
    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

    typedef Job::Model<DrawBackground> JobModel;
};


class DrawPostLayered {
public:
    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

    typedef Job::Model<DrawPostLayered> JobModel;
};

class ResetGLState {
public:
    void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

    typedef Job::Model<ResetGLState> JobModel;
};



class DrawSceneTask : public Task {
public:

    DrawSceneTask();
    ~DrawSceneTask();

    Jobs _jobs;

    virtual void run(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext);

};


// A map of ItemIDs allowing to create bucket lists of SHAPE type items which are filtered by their
// Material 
class ItemMaterialBucketMap : public std::map<model::MaterialFilter, ItemIDs, model::MaterialFilter::Less> {
public:

    ItemMaterialBucketMap() {}

    void insert(const ItemID& id, const model::MaterialKey& key);

    // standard builders allocating the main buckets
    void allocateStandardMaterialBuckets();
};
void materialSortItems(const SceneContextPointer& sceneContext, const RenderContextPointer& renderContext, const ItemIDsBounds& inItems, ItemIDsBounds& outItems);

}

#endif // hifi_render_Task_h
