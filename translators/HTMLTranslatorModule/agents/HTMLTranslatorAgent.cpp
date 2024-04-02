/*
Purpose: transform sc-model of the UI to a state that's translatable to HTML 
and launch the process of getting the full HTML component tree 
1. Traverse through the model: create relations from sc-nodes of components to their respective templates
    1.1 Use rules to check which templates should be used for a respective component class
    1.2 generate a relation
2. Generate the variables needed for the templates
    2.1 Check specs of the template: required variables, optional variables
    2.2 Generate variable map for the templates (lazy evaluation: <name, sc-link | sc-action | sc-template>)
        2.2.1 OPTION 1: Template spec will define sc-templates /sc-actions to get the value of the variables
            2.2.1.1 Run a search / run agent against each required variable, add it to the map, 
                    oportunistically complete with optional variables
        2.2.1 OPTION 2: (slow but extensible) We could check each parameter that the component has 
            2.2.1.1 does the parameter have an html translation rule? 
                    we should know how can it be integrated into the component template
            2.2.1.2 each parameter that has an html template gets inserted into a variable map
    2.3 We'll get a tree of templates that depend on each other (something like an execution plan for the templater)
        root template -> variable <name, (sc-template | sc-action) -> nested template -> variable <name, (...)>  >
3. Run template evaluation agent for the root of the tree
*/